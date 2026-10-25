#pragma once

#include <cstddef>

namespace rx::ast {
struct StructItem;
}

namespace rx::semantic {
enum class TypeKind { I32, U32, ISIZE, USIZE, BOOL, UNIT, NEVER, ARRAY, REF, BOX, VEC, STRUCT };


struct Type {
  TypeKind kind;
  const Type* target = nullptr;
  std::size_t length = 0;
  bool is_mutable = false;
  const ast::StructItem* declaration = nullptr;
};


bool isInteger(TypeKind kind);

bool sameType(const Type* left, const Type* right);
}
