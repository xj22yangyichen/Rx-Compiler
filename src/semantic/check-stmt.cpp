#include "analyzer.h"
#include <variant>

namespace rx::semantic {
namespace {
const ast::BlockExpr* asBlock(const ast::Expr& expression) {
  return std::get_if<ast::BlockExpr>(&expression.value);
}
}

void Analyzer::checkFunction(const ast::FunctionItem& item) {
  if (item.receiver.has_value() && self_struct_ == nullptr) {
    error(item.span, "a receiver is only allowed inside an impl");
    return;
  }
  current_function_ = &item;
  auto& signature = result_->functions[&item];
  signature.parameters.clear();
  const Type* saved_return = return_type_;
  return_type_ = make(TypeKind::UNIT);
  if (item.return_type != nullptr) {
    return_type_ = resolveType(*item.return_type);
  }
  pushScope();
  if (item.receiver.has_value()) {
    const Type* owner = structType(*self_struct_);
    const bool is_reference = item.receiver->is_reference;
    const Type* type = is_reference ? referenceType(owner, item.receiver->is_mutable) : owner;
    signature.receiver = type;
    scopes_.back().emplace("self", LocalVar{type, !is_reference && item.receiver->is_mutable, &*item.receiver});
  }
  for (const auto& parameter : item.parameters) {
    const Type* type = resolveType(*parameter.type);
    signature.parameters.push_back(type);
    scopes_.back().emplace(parameter.name, LocalVar{type, parameter.is_mutable, &parameter});
  }
  signature.result = return_type_;
  if (item.body != nullptr) {
    const ast::BlockExpr* body = asBlock(*item.body);
    if (body != nullptr) {
      const Type* type = checkBlock(*body, return_type_);
      record_expression(*item.body, type);
      record_coercion(*item.body, return_type_);
      if (type != nullptr && adjustTo(type, return_type_) == nullptr) {
        error(item.body->span, "the body does not match the declared return type");
      }
    }
  }
  popScope();
  return_type_ = saved_return;
  current_function_ = nullptr;
}

const Type* Analyzer::checkValue(const ast::Expr& expression, const Type* expected, bool* diverges) {
  bool child_diverges = false;
  const Type* type = nullptr;
  if (const auto* block = std::get_if<ast::BlockExpr>(&expression.value)) {
    type = checkBlock(*block, expected, &child_diverges);
  } else {
    type = checkExpr(expression, expected);
  }
  record_expression(expression, type);
  if (type != nullptr && type->kind == TypeKind::NEVER) {
    child_diverges = true;
  }
  if (type != nullptr && expected != nullptr) {
    coerce(expression, type, expected);
  }
  if (diverges != nullptr) {
    *diverges = child_diverges;
  }
  return type;
}

const Type* Analyzer::checkBlock(const ast::BlockExpr& block, const Type* expected, bool* diverges) {
  pushScope();
  bool block_diverges = false;
  for (const auto& statement : block.statements) {
    bool statement_diverges = false;
    checkStmt(*statement, &statement_diverges);
    if (statement_diverges) {
      block_diverges = true;
    }
  }
  const Type* result = nullptr;
  if (block.tail != nullptr) {
    bool tail_diverges = false;
    result = checkValue(*block.tail, expected, &tail_diverges);
    if (tail_diverges) {
      block_diverges = true;
    }
  } else {
    result = block_diverges ? make(TypeKind::NEVER) : make(TypeKind::UNIT);
  }
  popScope();
  if (diverges != nullptr) {
    *diverges = block_diverges;
  }
  result_->block_divergence[&block] = block_diverges;
  return result;
}

const Type* Analyzer::checkStmt(const ast::Stmt& statement, bool* diverges) {
  return std::visit([&](const auto& value) -> const Type* {
    using Node = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<Node, ast::EmptyStmt>) {
      if (diverges != nullptr) {
        *diverges = false;
      }
      return make(TypeKind::UNIT);
    } else if constexpr (std::is_same_v<Node, ast::LetStmt>) {
      return checkLet(value, diverges);
    } else {
      bool child_diverges = false;
      const Type* type = checkValue(*value.expression, nullptr, &child_diverges);
      if (!value.has_semicolon && type != nullptr && type->kind != TypeKind::NEVER) {
        if (adjustTo(type, make(TypeKind::UNIT)) == nullptr) {
          error(value.expression->span, "a non-final statement must have unit type");
        }
      }
      if (diverges != nullptr) {
        *diverges = child_diverges;
      }
      return type;
    }
  }, statement.value);
}

const Type* Analyzer::checkLet(const ast::LetStmt& binding, bool* diverges) {
  const Type* declared = binding.type != nullptr ? resolveType(*binding.type) : nullptr;
  bool child_diverges = false;
  const Type* value = checkValue(*binding.init, declared, &child_diverges);
  const Type* result = declared != nullptr ? declared : value;
  LocalVar local{result, binding.is_mutable, &binding};
  result_->locals[&binding] = local;
  scopes_.back().emplace(binding.name, local);
  if (diverges != nullptr) {
    *diverges = child_diverges;
  }
  return make(TypeKind::UNIT);
}
}
