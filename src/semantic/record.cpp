#include "analyzer.h"
#include <utility>

namespace rx::semantic {
void Analyzer::record_expression(const ast::Expr& expression, const Type* type) {
  const auto* block = std::get_if<ast::BlockExpr>(&expression.value);
  if (block != nullptr && block->tail != nullptr) {
    const auto& tail = result_->expressions.at(block->tail.get());
    type = tail.type;
    if (!tail.adjustments.empty()) {
      type = tail.adjustments.back().target;
    }
  }
  result_->expressions[&expression].type = type;
}

void Analyzer::record_derefs(const ast::Expr& expression, std::size_t count) {
  auto& info = result_->expressions[&expression];
  const Type* type = info.type;
  info.adjustments.clear();
  for (std::size_t index = 0; index < count; ++index) {
    type = derefOnce(type);
    if (type == nullptr) {
      return;
    }
    info.adjustments.push_back({AdjustmentKind::DEREFERENCE, type});
  }
}

void Analyzer::record_coercion(const ast::Expr& expression, const Type* to) {
  auto& info = result_->expressions[&expression];
  const Type* from = info.type;
  if (!info.adjustments.empty()) {
    from = info.adjustments.back().target;
  }
  if (from == nullptr || to == nullptr || sameType(from, to)) {
    return;
  }
  if (adjustTo(from, to) == nullptr) {
    return;
  }
  if (from->kind == TypeKind::NEVER) {
    info.adjustments.push_back({AdjustmentKind::NEVER, to});
    return;
  }
  const Type* inner = from;
  do {
    inner = derefOnce(inner);
    info.adjustments.push_back({AdjustmentKind::DEREFERENCE, inner});
  } while (!sameType(inner, to->target));
  AdjustmentKind kind = AdjustmentKind::BORROW;
  if (to->is_mutable) {
    kind = AdjustmentKind::BORROW_MUT;
  }
  info.adjustments.push_back({kind, to});
}

void Analyzer::record_method(const ast::MethodCallExpr& call, CallInfo info,
  std::size_t derefs, int variant, const Type* candidate) {
  const Type* type = result_->expressions[call.receiver.get()].type;
  for (std::size_t index = 0; index < derefs; ++index) {
    type = derefOnce(type);
    info.receiver_adjustments.push_back({AdjustmentKind::DEREFERENCE, type});
  }
  if (variant != 0) {
    AdjustmentKind kind = AdjustmentKind::BORROW;
    if (variant == 2) {
      kind = AdjustmentKind::BORROW_MUT;
    }
    info.receiver_adjustments.push_back({kind, candidate});
  }
  info.receiver = candidate;
  result_->methods[&call] = std::move(info);
}
}
