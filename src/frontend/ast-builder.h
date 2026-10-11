#pragma once

#include "../ast/ast.h"
#include "RxParser.h"

namespace rx {
class AstBuilder
{
public:
  // entrance of the ast builder
  ast::Root build_root(antlr4::ParserRuleContext* context);

private:
  using Context = antlr4::ParserRuleContext;
  ast::ItemPtr build_item(Context* context);
  ast::TypePtr build_type(Context* context);
  ast::StmtPtr build_stmt(Context* context);
  ast::ExprPtr build_expr(Context* context);

  // atomic expression
  ast::ExprPtr build_primary(Context* context);
  // binary expression and assgnment expression
  ast::ExprPtr build_chain(Context* context);
  // unary expression
  ast::ExprPtr build_unary(Context* context);
  // cast expression
  ast::ExprPtr build_cast(Context* context);
  // postfix expression, including function call, method call, field access, array access
  ast::ExprPtr build_postfix(Context* context);

  ast::ExprPtr build_block(RxParser::BlockExpressionContext* context);
  ast::ExprPtr build_literal(antlr4::Token* token);
  ast::Path build_path(Context* context);
  ast::PathSegment build_segment(Context* context);
  std::vector<ast::ExprPtr> build_arguments(RxParser::CallArgumentsContext* context);
};

ast::Span sourceSpan(antlr4::ParserRuleContext* context);
ast::Span tokenSpan(antlr4::Token* token);
}
