#include "ast-builder.h"
#include <stdexcept>
#include <algorithm>

namespace rx {
using namespace ast;
namespace {
using Context = antlr4::ParserRuleContext;

Context* rule(antlr4::tree::ParseTree* tree) {
  return static_cast<Context*>(tree);
}

Span joinedSpan(const Span& left, const Span& right) {
  auto span = left;
  span.end = right.end;
  return span;
}

bool isOperator(Context* context) {
  switch (context->getRuleIndex()) {
    case RxParser::RuleAssignmentOperator:
    case RxParser::RuleComparisonExceptLt:
    case RxParser::RuleAdditiveOperator:
    case RxParser::RuleMultiplicativeOperator:
    case RxParser::RuleShiftRight:
      return true;
    default:
      return false;
  }
}

Op binaryOp(const std::string& text) {
  for (auto value = static_cast<int>(Op::ADD); value <= static_cast<int>(Op::SHIFT_RIGHT_ASSIGN); ++value) {
    auto op = static_cast<Op>(value);
    if (text == opText(op)) {
      return op;
    }
  }
  throw std::logic_error("unknown binary operator: " + text);
}
}

Span tokenSpan(antlr4::Token* token) {
  const auto begin = token->getStartIndex();
  auto end = begin;
  if (token->getType() != antlr4::Token::EOF) {
    end = token->getStopIndex() + 1;
  }
  return {begin, end, token->getLine(), token->getCharPositionInLine() + 1};
}

Span sourceSpan(antlr4::ParserRuleContext* context) {
  auto span = tokenSpan(context->getStart());
  if (context->getStop()) {
    span.end = std::max(span.begin, tokenSpan(context->getStop()).end);
  }
  return span;
}

Root AstBuilder::build_root(Context* context) {
  switch (context->getRuleIndex()) {
    case RxParser::RuleCrate: {
      auto crate = std::make_unique<Crate>();
      crate->span = sourceSpan(context);
      for (auto* item : static_cast<RxParser::CrateContext*>(context)->item()) {
        crate->items.push_back(build_item(item));
      }
      return crate;
    }
    case RxParser::RuleItem:
      return build_item(context);
    case RxParser::RuleTypeRef:
      return build_type(context);
    case RxParser::RuleLetStatement:
      return build_stmt(context);
    default:
      return build_expr(context);
  }
}

PathSegment AstBuilder::build_segment(Context* context) {
  PathSegment segment{sourceSpan(context), {}, {}};
  for (auto* child : context->children) {
    if (auto* name = dynamic_cast<RxParser::PathIdentSegmentContext*>(child)) {
      segment.name = name->getText();
    } else if (auto* args = dynamic_cast<RxParser::GenericArgsContext*>(child)) {
      for (auto* arg : args->genericArg()) {
        if (arg->typeRef()) {
          segment.arguments.push_back(build_type(arg->typeRef()));
        }
      }
    }
  }
  return segment;
}

Path AstBuilder::build_path(Context* context) {
  Path path;
  for (auto* child : context->children) {
    if (auto* segment = dynamic_cast<RxParser::TypePathSegmentContext*>(child)) {
      path.push_back(build_segment(segment));
    } else if (auto* segment = dynamic_cast<RxParser::PathExprSegmentContext*>(child)) {
      path.push_back(build_segment(segment));
    }
  }
  return path;
}

TypePtr AstBuilder::build_type(Context* context) {
  const auto span = sourceSpan(context);
  if (auto* type = dynamic_cast<RxParser::TypeRefContext*>(context)) {
    if (type->typeRef()) {
      auto result = build_type(type->typeRef());
      result->span = span;
      return result;
    }
    if (type->typePath()) {
      return makeNode<Type>(span, PathType{build_path(type->typePath())});
    }
    if (type->referenceType()) {
      return build_type(type->referenceType());
    }
    if (type->arrayType()) {
      return build_type(type->arrayType());
    }
    return makeNode<Type>(span, UnitType{});
  }
  if (auto* type = dynamic_cast<RxParser::ArrayTypeContext*>(context)) {
    return makeNode<Type>(span, ArrayType{build_type(type->typeRef()), build_expr(type->constValue())});
  }
  if (auto* type = dynamic_cast<RxParser::ReferenceTypeContext*>(context)) {
    auto inner_span = span;
    if (type->ANDAND()) {
      ++inner_span.begin;
      ++inner_span.column;
    }
    auto result =
      makeNode<Type>(inner_span, RefType{type->MUT() != nullptr, build_type(type->typeRef())});
    if (type->ANDAND()) {
      result = makeNode<Type>(span, RefType{false, std::move(result)});
    }
    return result;
  }
  auto* type = static_cast<RxParser::ClosedCastTypeContext*>(context);
  if (type->arrayType()) {
    return build_type(type->arrayType());
  }
  if (type->LPAREN()) {
    TypePtr result;
    if (type->typeRef()) {
      result = build_type(type->typeRef());
    } else {
      result = makeNode<Type>(span, UnitType{});
    }
    result->span = span;
    return result;
  }
  if (type->closedCastType()) {
    auto inner_span = span;
    if (type->ANDAND()) {
      ++inner_span.begin;
      ++inner_span.column;
    }
    auto result = makeNode<Type>(
      inner_span, RefType{type->MUT() != nullptr, build_type(type->closedCastType())});
    if (type->ANDAND()) {
      result = makeNode<Type>(span, RefType{false, std::move(result)});
    }
    return result;
  }
  Path path;
  for (auto* segment : type->typePathSegment()) {
    path.push_back(build_segment(segment));
  }
  auto final_segment = build_segment(type);
  final_segment.span = sourceSpan(type->pathIdentSegment());
  final_segment.span.end = sourceSpan(type->genericArgs()).end;
  path.push_back(std::move(final_segment));
  return makeNode<Type>(span, PathType{std::move(path)});
}

ItemPtr AstBuilder::build_item(Context* context) {
  if (auto* item = dynamic_cast<RxParser::ItemContext*>(context)) {
    return build_item(rule(item->children.at(0)));
  }
  if (auto* item = dynamic_cast<RxParser::AssociatedItemContext*>(context)) {
    return build_item(rule(item->children.at(0)));
  }
  const auto span = sourceSpan(context);
  if (dynamic_cast<RxParser::UseDeclarationContext*>(context)) {
    return makeNode<Item>(span, UseItem{});
  }
  if (auto* function = dynamic_cast<RxParser::FunctionDefinitionContext*>(context)) {
    FunctionItem value;
    value.name = function->identifier()->getText();
    if (auto* params = function->functionParameters()) {
      if (auto* self = params->selfParam()) {
        value.receiver = Receiver{sourceSpan(self), self->AMP() != nullptr, self->MUT() != nullptr};
      }
      for (auto* param : params->functionParam()) {
        auto* binding = param->identifierBinding();
        value.parameters.push_back({sourceSpan(param), binding->identifier()->getText(),
          binding->MUT() != nullptr, build_type(param->typeRef())});
      }
    }
    if (function->typeRef()) {
      value.return_type = build_type(function->typeRef());
    }
    value.body = build_block(function->blockExpression());
    return makeNode<Item>(span, std::move(value));
  }
  if (auto* structure = dynamic_cast<RxParser::StructDefinitionContext*>(context)) {
    StructItem value;
    value.name = structure->identifier()->getText();
    for (auto* attribute : structure->outerAttribute()) {
      for (auto* derive : attribute->deriveName()) {
        value.derives.push_back(derive->getText());
      }
    }
    for (auto* field : structure->structField()) {
      value.fields.push_back({sourceSpan(field),
        field->identifier()->getText(), build_type(field->typeRef())});
    }
    return makeNode<Item>(span, std::move(value));
  }
  if (auto* constant = dynamic_cast<RxParser::ConstantItemContext*>(context)) {
    return makeNode<Item>(span, ConstItem{constant->identifier()->getText(),
      build_type(constant->typeRef()), build_expr(constant->constValue())});
  }
  auto* impl = static_cast<RxParser::InherentImplContext*>(context);
  ImplItem value{build_type(impl->typeRef()), {}};
  for (auto* item : impl->associatedItem()) {
    value.items.push_back(build_item(item));
  }
  return makeNode<Item>(span, std::move(value));
}

StmtPtr AstBuilder::build_stmt(Context* context) {
  const auto span = sourceSpan(context);
  if (auto* binding = dynamic_cast<RxParser::LetStatementContext*>(context)) {
    auto* name = binding->identifierBinding();
    TypePtr type;
    if (binding->typeRef()) {
      type = build_type(binding->typeRef());
    }
    return makeNode<Stmt>(span, LetStmt{name->identifier()->getText(),
      name->MUT() != nullptr, std::move(type), build_expr(binding->expression())});
  }
  auto* statement = static_cast<RxParser::StatementContext*>(context);
  if (statement->letStatement()) {
    return build_stmt(statement->letStatement());
  }
  if (statement->expressionWithBlock()) {
    return makeNode<Stmt>(span,
      ExprStmt{build_expr(statement->expressionWithBlock()), statement->SEMI() != nullptr});
  }
  if (statement->statementExpression()) {
    return makeNode<Stmt>(span, ExprStmt{build_expr(statement->statementExpression()), true});
  }
  return makeNode<Stmt>(span, EmptyStmt{});
}

ExprPtr AstBuilder::build_block(RxParser::BlockExpressionContext* context) {
  BlockExpr value;
  for (auto* statement : context->statement()) {
    value.statements.push_back(build_stmt(statement));
  }
  if (context->statementExpression()) {
    value.tail = build_expr(context->statementExpression());
  } else if (!value.statements.empty()) {
    // ANTLR may represent a final block/if/loop as a semicolon-free statement.
    auto* last = std::get_if<ExprStmt>(&value.statements.back()->value);
    if (last && !last->has_semicolon) {
      value.tail = std::move(last->expression);
      value.statements.pop_back();
    }
  }
  return makeNode<Expr>(sourceSpan(context), std::move(value));
}

ExprPtr AstBuilder::build_literal(antlr4::Token* token) {
  const auto span = tokenSpan(token);
  if (token->getType() == RxParser::TRUE || token->getType() == RxParser::FALSE) {
    return makeNode<Expr>(span, BoolExpr{token->getType() == RxParser::TRUE});
  }
  IntegerExpr value{token->getText(), token->getText(), 10, {}};
  for (const std::string suffix : {"isize", "usize", "i32", "u32"}) {
    if (value.digits.size() >= suffix.size() &&
      value.digits.compare(value.digits.size() - suffix.size(), suffix.size(), suffix) == 0) {
      value.suffix = suffix;
      value.digits.resize(value.digits.size() - suffix.size());
      break;
    }
  }
  if (value.digits.size() >= 2 && value.digits[0] == '0') {
    switch (value.digits[1]) {
      case 'b':
        value.base = 2;
        break;
      case 'o':
        value.base = 8;
        break;
      case 'x':
        value.base = 16;
        break;
    }
    if (value.base != 10) {
      value.digits.erase(0, 2);
    }
  }
  value.digits.erase(std::remove(value.digits.begin(), value.digits.end(), '_'), value.digits.end());
  return makeNode<Expr>(span, std::move(value));
}

ExprPtr AstBuilder::build_chain(Context* context) {
  ExprPtr result;
  std::string operator_text;
  for (auto* child : context->children) {
    auto* child_rule = dynamic_cast<Context*>(child);
    if (!child_rule || isOperator(child_rule)) {
      operator_text += child->getText();
      continue;
    }
    auto right = build_expr(child_rule);
    if (!result) {
      result = std::move(right);
      continue;
    }
    auto span = joinedSpan(result->span, right->span);
    const auto op = binaryOp(operator_text);
    if (op >= Op::ASSIGN) {
      result = makeNode<Expr>(span, AssignExpr{op, std::move(result), std::move(right)});
    } else {
      result = makeNode<Expr>(span, BinaryExpr{op, std::move(result), std::move(right)});
    }
    operator_text.clear();
  }
  return result;
}

ExprPtr AstBuilder::build_unary(Context* context) {
  if (context->children.size() == 1) {
    return build_expr(rule(context->children.front()));
  }
  const auto text = context->children.front()->getText();
  auto operand = build_expr(rule(context->children.at(1)));
  auto span = sourceSpan(context);
  if (text == "&&" || text == "&&mut") {
    auto inner_span = span;
    ++inner_span.begin;
    ++inner_span.column;
    auto inner_op = Op::BORROW;
    if (text == "&&mut") {
      inner_op = Op::BORROW_MUT;
    }
    operand = makeNode<Expr>(inner_span, UnaryExpr{inner_op, std::move(operand)});
    return makeNode<Expr>(span, UnaryExpr{Op::BORROW, std::move(operand)});
  }
  Op op = Op::BORROW_MUT;
  if (text == "-") {
    op = Op::NEGATE;
  } else if (text == "!") {
    op = Op::NOT;
  } else if (text == "*") {
    op = Op::DEREFERENCE;
  } else if (text == "&") {
    op = Op::BORROW;
  }
  return makeNode<Expr>(span, UnaryExpr{op, std::move(operand)});
}

ExprPtr AstBuilder::build_cast(Context* context) {
  auto result = build_expr(rule(context->children.front()));
  for (std::size_t i = 1; i < context->children.size(); i += 2) {
    auto* type = rule(context->children.at(i + 1));
    auto span = joinedSpan(result->span, sourceSpan(type));
    result = makeNode<Expr>(span, CastExpr{std::move(result), build_type(type)});
  }
  return result;
}

std::vector<ExprPtr> AstBuilder::build_arguments(RxParser::CallArgumentsContext* context) {
  std::vector<ExprPtr> result;
  for (auto* expression : context->expression()) {
    result.push_back(build_expr(expression));
  }
  return result;
}

ExprPtr AstBuilder::build_postfix(Context* context) {
  auto result = build_expr(rule(context->children.front()));
  for (std::size_t i = 1; i < context->children.size(); ++i) {
    auto* suffix = rule(context->children[i]);
    const auto span = joinedSpan(result->span, sourceSpan(suffix));
    if (auto* postfix = dynamic_cast<RxParser::PostfixSuffixContext*>(suffix)) {
      if (postfix->callArguments()) {
        result = makeNode<Expr>(span, CallExpr{std::move(result), build_arguments(postfix->callArguments())});
        continue;
      }
      if (postfix->expression()) {
        result = makeNode<Expr>(span, IndexExpr{std::move(result), build_expr(postfix->expression())});
        continue;
      }
      suffix = postfix->dotSuffix();
    }
    auto* dot = static_cast<RxParser::DotSuffixContext*>(suffix);
    if (dot->callArguments()) {
      result = makeNode<Expr>(span, MethodCallExpr{std::move(result),
        build_segment(dot->pathExprSegment()), build_arguments(dot->callArguments())});
    } else {
      result = makeNode<Expr>(span, FieldExpr{std::move(result), dot->identifier()->getText()});
    }
  }
  return result;
}

ExprPtr AstBuilder::build_primary(Context* context) {
  const auto span = sourceSpan(context);
  auto* first = context->children.front();
  if (auto* token = dynamic_cast<antlr4::tree::TerminalNode*>(first)) {
    const auto kind = token->getSymbol()->getType();
    if (kind == RxParser::INTEGER_LITERAL || kind == RxParser::TRUE || kind == RxParser::FALSE) {
      return build_literal(token->getSymbol());
    }
    if (kind == RxParser::LPAREN) {
      if (context->children.size() == 2) {
        return makeNode<Expr>(span, UnitExpr{});
      }
      auto result = build_expr(rule(context->children.at(1)));
      result->span = span;
      return result;
    }
    if (kind == RxParser::CONTINUE) {
      return makeNode<Expr>(span, ContinueExpr{});
    }
    if (kind == RxParser::BREAK || kind == RxParser::RETURN) {
      ExprPtr value;
      if (context->children.size() > 1) {
        value = build_expr(rule(context->children.at(1)));
      }
      if (kind == RxParser::BREAK) {
        return makeNode<Expr>(span, BreakExpr{std::move(value)});
      }
      return makeNode<Expr>(span, ReturnExpr{std::move(value)});
    }
    if (kind == RxParser::LOOP) {
      return makeNode<Expr>(span, LoopExpr{build_expr(rule(context->children.at(1)))});
    }
    if (kind == RxParser::WHILE) {
      return makeNode<Expr>(span, WhileExpr{
        build_expr(rule(context->children.at(1))), build_expr(rule(context->children.at(2)))});
    }
    return makeNode<Expr>(span, UnaryExpr{Op::NEGATE, build_expr(rule(context->children.at(1)))});
  }
  if (auto* path = dynamic_cast<RxParser::PathInExpressionContext*>(first)) {
    if (context->children.size() == 1) {
      return makeNode<Expr>(span, PathExpr{build_path(path)});
    }
    StructExpr value{build_path(path), {}};
    for (auto* child : context->children) {
      if (auto* fields = dynamic_cast<RxParser::StructExprFieldsContext*>(child)) {
        for (auto* field : fields->structExprField()) {
          value.fields.push_back(
            {sourceSpan(field), field->identifier()->getText(), build_expr(field->expression())});
        }
      }
    }
    return makeNode<Expr>(span, std::move(value));
  }
  return build_expr(rule(first));
}

ExprPtr AstBuilder::build_expr(Context* context) {
  switch (context->getRuleIndex()) {
    case RxParser::RuleExpression:
    case RxParser::RuleConditionExpression:
    case RxParser::RuleConditionBreakExpression:
    case RxParser::RuleStatementExpression:
    case RxParser::RuleAssignmentExpression:
    case RxParser::RuleConditionAssignmentExpression:
    case RxParser::RuleConditionBreakAssignmentExpression:
    case RxParser::RuleStatementAssignmentExpression:
    case RxParser::RuleLogicalOrExpression:
    case RxParser::RuleConditionLogicalOrExpression:
    case RxParser::RuleConditionBreakLogicalOrExpression:
    case RxParser::RuleStatementLogicalOrExpression:
    case RxParser::RuleLogicalAndExpression:
    case RxParser::RuleConditionLogicalAndExpression:
    case RxParser::RuleConditionBreakLogicalAndExpression:
    case RxParser::RuleStatementLogicalAndExpression:
    case RxParser::RuleComparisonExpression:
    case RxParser::RuleConditionComparisonExpression:
    case RxParser::RuleConditionBreakComparisonExpression:
    case RxParser::RuleStatementComparisonExpression:
    case RxParser::RuleBitOrExpression:
    case RxParser::RuleConditionBitOrExpression:
    case RxParser::RuleConditionBreakBitOrExpression:
    case RxParser::RuleStatementBitOrExpression:
    case RxParser::RuleClosedBitOrExpression:
    case RxParser::RuleConditionClosedBitOrExpression:
    case RxParser::RuleConditionBreakClosedBitOrExpression:
    case RxParser::RuleStatementClosedBitOrExpression:
    case RxParser::RuleBitXorExpression:
    case RxParser::RuleConditionBitXorExpression:
    case RxParser::RuleConditionBreakBitXorExpression:
    case RxParser::RuleStatementBitXorExpression:
    case RxParser::RuleClosedBitXorExpression:
    case RxParser::RuleConditionClosedBitXorExpression:
    case RxParser::RuleConditionBreakClosedBitXorExpression:
    case RxParser::RuleStatementClosedBitXorExpression:
    case RxParser::RuleBitAndExpression:
    case RxParser::RuleConditionBitAndExpression:
    case RxParser::RuleConditionBreakBitAndExpression:
    case RxParser::RuleStatementBitAndExpression:
    case RxParser::RuleClosedBitAndExpression:
    case RxParser::RuleConditionClosedBitAndExpression:
    case RxParser::RuleConditionBreakClosedBitAndExpression:
    case RxParser::RuleStatementClosedBitAndExpression:
    case RxParser::RuleShiftExpression:
    case RxParser::RuleConditionShiftExpression:
    case RxParser::RuleConditionBreakShiftExpression:
    case RxParser::RuleStatementShiftExpression:
    case RxParser::RuleClosedShiftExpression:
    case RxParser::RuleConditionClosedShiftExpression:
    case RxParser::RuleConditionBreakClosedShiftExpression:
    case RxParser::RuleStatementClosedShiftExpression:
    case RxParser::RuleAdditiveExpression:
    case RxParser::RuleConditionAdditiveExpression:
    case RxParser::RuleConditionBreakAdditiveExpression:
    case RxParser::RuleStatementAdditiveExpression:
    case RxParser::RuleClosedAdditiveExpression:
    case RxParser::RuleConditionClosedAdditiveExpression:
    case RxParser::RuleConditionBreakClosedAdditiveExpression:
    case RxParser::RuleStatementClosedAdditiveExpression:
    case RxParser::RuleMultiplicativeExpression:
    case RxParser::RuleConditionMultiplicativeExpression:
    case RxParser::RuleConditionBreakMultiplicativeExpression:
    case RxParser::RuleStatementMultiplicativeExpression:
    case RxParser::RuleClosedMultiplicativeExpression:
    case RxParser::RuleConditionClosedMultiplicativeExpression:
    case RxParser::RuleConditionBreakClosedMultiplicativeExpression:
    case RxParser::RuleStatementClosedMultiplicativeExpression:
      return build_chain(context);
    case RxParser::RuleUnaryExpression:
    case RxParser::RuleConditionUnaryExpression:
    case RxParser::RuleConditionBreakUnaryExpression:
    case RxParser::RuleStatementUnaryExpression:
      return build_unary(context);
    case RxParser::RuleCastExpression:
    case RxParser::RuleConditionCastExpression:
    case RxParser::RuleConditionBreakCastExpression:
    case RxParser::RuleStatementCastExpression:
    case RxParser::RuleClosedCastExpression:
    case RxParser::RuleConditionClosedCastExpression:
    case RxParser::RuleConditionBreakClosedCastExpression:
    case RxParser::RuleStatementClosedCastExpression:
      return build_cast(context);
    case RxParser::RulePostfixExpression:
    case RxParser::RuleConditionPostfixExpression:
    case RxParser::RuleConditionBreakPostfixExpression:
    case RxParser::RuleStatementPostfixExpression:
      return build_postfix(context);
    case RxParser::RulePathInExpression:
      return makeNode<Expr>(sourceSpan(context), PathExpr{build_path(context)});
    case RxParser::RuleBlockExpression:
      return build_block(static_cast<RxParser::BlockExpressionContext*>(context));
    case RxParser::RuleIfExpression: {
      auto* branch = static_cast<RxParser::IfExpressionContext*>(context);
      auto blocks = branch->blockExpression();
      IfExpr value{build_expr(branch->conditionExpression()), build_block(blocks.at(0)), nullptr};
      if (branch->ifExpression()) {
        value.else_branch = build_expr(branch->ifExpression());
      } else if (blocks.size() > 1) {
        value.else_branch = build_block(blocks.at(1));
      }
      return makeNode<Expr>(sourceSpan(context), std::move(value));
    }
    case RxParser::RuleArrayExpression: {
      auto* array = static_cast<RxParser::ArrayExpressionContext*>(context);
      if (array->constValue()) {
        return makeNode<Expr>(sourceSpan(context),
          RepeatExpr{build_expr(array->expression(0)), build_expr(array->constValue())});
      }
      ArrayExpr value;
      for (auto* element : array->expression()) {
        value.elements.push_back(build_expr(element));
      }
      return makeNode<Expr>(sourceSpan(context), std::move(value));
    }
    default:
      return build_primary(context);
  }
}
}
