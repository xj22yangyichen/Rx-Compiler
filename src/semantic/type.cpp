#include "type.h"

namespace rx::semantic {
bool isInteger(TypeKind kind) {
  return kind == TypeKind::I32 || kind == TypeKind::U32 || kind == TypeKind::ISIZE || kind == TypeKind::USIZE;
}

bool sameType(const Type* left, const Type* right) {
  if (left == right) {
    return true;
  }
  if (left->kind != right->kind) {
    return false;
  }
  if (left->kind == TypeKind::STRUCT) {
    return left->declaration == right->declaration;
  }
  if (left->is_mutable != right->is_mutable || left->length != right->length) {
    return false;
  }
  if (left->target == nullptr || right->target == nullptr) {
    return left->target == right->target;
  }
  return sameType(left->target, right->target);
}
}
