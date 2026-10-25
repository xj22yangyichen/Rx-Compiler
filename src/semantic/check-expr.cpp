#include "analyzer.h"
#include <set>
#include <vector>
#include <string>
#include <variant>

namespace rx::semantic {
namespace {
const ast::BlockExpr* asBlock(const ast::Expr& expression) {
  return std::get_if<ast::BlockExpr>(&expression.value);
}

bool isSignedKind(TypeKind kind) {
  return kind == TypeKind::I32 || kind == TypeKind::ISIZE;
}

bool isShift(ast::Op op) {
  return op == ast::Op::SHIFT_LEFT || op == ast::Op::SHIFT_RIGHT;
}

bool isComparison(ast::Op op) {
  return op == ast::Op::LESS || op == ast::Op::LESS_EQUAL
    || op == ast::Op::GREATER || op == ast::Op::GREATER_EQUAL;
}

std::optional<Place> advancePlace(std::optional<Place> place, std::size_t steps) {
  for (std::size_t step = 0; step < steps && place.has_value(); ++step) {
    const Type* type = place->type;
    const bool shared = place->is_borrowed_shared || (type->kind == TypeKind::REF && !type->is_mutable);
    if (type->kind == TypeKind::REF) {
      place = Place{type->target, !shared, shared};
    } else if (type->kind == TypeKind::BOX) {
      place = Place{type->target, place->is_mutable && !shared, shared};
    } else {
      return std::nullopt;
    }
  }
  return place;
}
}

const Type* Analyzer::derefOnce(const Type* type) {
  if (type == nullptr) {
    return nullptr;
  }
  if (type->kind == TypeKind::REF || type->kind == TypeKind::BOX) {
    return type->target;
  }
  return nullptr;
}

void Analyzer::requireMutable(const ast::Span& span, const Place& place, const std::string& what) {
  if (!place.is_mutable) {
    error(span, what + " requires a mutable place");
  }
}

bool Analyzer::expectBool(const ast::Expr& expression, const std::string& context) {
  const Type* type = checkExpr(expression, make(TypeKind::BOOL));
  if (type == nullptr) {
    return false;
  }
  if (type->kind != TypeKind::BOOL) {
    error(expression.span, context + " requires a bool");
    return false;
  }
  return true;
}

Place Analyzer::narrowPlace(Place place) {
  if (place.type == nullptr) {
    return place;
  }
  if (place.is_borrowed_shared && place.type->kind == TypeKind::REF && place.type->is_mutable) {
    place.type = referenceType(place.type->target, false);
  }
  return place;
}

const Type* Analyzer::fieldOf(const Type* type, const std::string& name, const ast::Span& span) {
  const Type* current = type;
  while (current != nullptr && (current->kind == TypeKind::REF || current->kind == TypeKind::BOX)) {
    current = current->target;
  }
  if (current == nullptr || current->kind != TypeKind::STRUCT) {
    error(span, "this type has no named fields");
    return nullptr;
  }
  const StructInfo* info = structInfo(current->declaration);
  if (info == nullptr) {
    return nullptr;
  }
  for (std::size_t index = 0; index < info->declaration->fields.size(); ++index) {
    if (info->declaration->fields[index].name == name) {
      return info->field_types[index];
    }
  }
  error(span, "unknown field");
  return nullptr;
}

const FunctionInfo* Analyzer::findInherent(const Type* candidate, const std::string& name) {
  const Type* shape = candidate->kind == TypeKind::REF ? candidate->target : candidate;
  if (shape == nullptr || shape->kind != TypeKind::STRUCT) {
    return nullptr;
  }
  const StructInfo* info = structInfo(shape->declaration);
  if (info == nullptr) {
    return nullptr;
  }
  auto found = info->functions.find(name);
  if (found == info->functions.end()) {
    return nullptr;
  }
  const ast::FunctionItem& method = *found->second.declaration;
  if (!method.receiver.has_value()) {
    return nullptr;
  }
  const Type* owner = structType(*info);
  const Type* expected = method.receiver->is_reference ? referenceType(owner, method.receiver->is_mutable) : owner;
  return sameType(expected, candidate) ? &found->second : nullptr;
}

const Type* Analyzer::adjustTo(const Type* from, const Type* to) {
  if (from == nullptr || to == nullptr) {
    return to;
  }
  if (sameType(from, to)) {
    return to;
  }
  if (from->kind == TypeKind::NEVER) {
    return to;
  }
  if (from->kind != TypeKind::REF || to->kind != TypeKind::REF) {
    return nullptr;
  }
  if (from->is_mutable && !to->is_mutable && sameType(from->target, to->target)) {
    return to;
  }
  bool shared = !from->is_mutable;
  const Type* inner = from->target;
  while (inner != nullptr) {
    if (sameType(inner, to->target)) {
      return to->is_mutable && shared ? nullptr : to;
    }
    if (inner->kind == TypeKind::REF) {
      if (!inner->is_mutable) {
        shared = true;
      }
      inner = inner->target;
      continue;
    }
    if (inner->kind == TypeKind::BOX) {
      inner = inner->target;
      continue;
    }
    return nullptr;
  }
  return nullptr;
}

bool Analyzer::coerce(const ast::Expr& expression, const Type* from, const Type* to) {
  if (adjustTo(from, to) != nullptr) {
    return true;
  }
  error(expression.span, "type mismatch");
  return false;
}

const Type* Analyzer::lubOf(const std::vector<const Type*>& types) {
  std::vector<const Type*> seen;
  const Type* result = nullptr;
  for (const Type* type : types) {
    if (type == nullptr) {
      return nullptr;
    }
    if (type->kind == TypeKind::NEVER) {
      continue;
    }
    if (result == nullptr) {
      result = type;
      seen.push_back(type);
      continue;
    }
    if (adjustTo(type, result) != nullptr) {
      seen.push_back(type);
      continue;
    }
    if (adjustTo(result, type) != nullptr) {
      bool all_adjust = true;
      for (const Type* earlier : seen) {
        if (adjustTo(earlier, type) == nullptr) {
          all_adjust = false;
        }
      }
      if (all_adjust) {
        result = type;
        seen.push_back(type);
        continue;
      }
    }
    return nullptr;
  }
  if (result == nullptr && !types.empty()) {
    return make(TypeKind::NEVER);
  }
  return result;
}

const Type* Analyzer::checkIntegerLiteral(const ast::IntegerExpr& literal, const Type* expected) {
  if (!literal.suffix.empty()) {
    const Type* type = integerType(literal.suffix);
    return type == nullptr ? make(TypeKind::I32) : type;
  }
  if (expected != nullptr && isInteger(expected->kind)) {
    return expected;
  }
  return make(TypeKind::I32);
}

const Type* Analyzer::checkPathExpr(const ast::PathExpr& path) {
  if (path.path.size() == 1) {
    if (LocalVar* local = lookupLocal(path.path.front().name)) {
      return local->type;
    }
  }
  const ast::ConstItem* item = resolveConstantPath(path.path);
  if (item != nullptr) {
    return resolveType(*item->type);
  }
  error(path.path.front().span, "unresolved name");
  return nullptr;
}

const Type* Analyzer::checkUnary(const ast::UnaryExpr& unary) {
  if (unary.op == ast::Op::BORROW || unary.op == ast::Op::BORROW_MUT) {
    const bool is_mutable = unary.op == ast::Op::BORROW_MUT;
    auto place = checkPlace(*unary.operand);
    if (!place.has_value()) {
      const Type* value = checkExpr(*unary.operand, nullptr);
      return value == nullptr ? nullptr : referenceType(value, is_mutable);
    }
    if (is_mutable) {
      requireMutable(unary.operand->span, *place, "a mutable borrow");
    }
    return referenceType(place->type, is_mutable);
  }
  if (unary.op == ast::Op::DEREFERENCE) {
    auto place = placeOfUnary(unary);
    return place.has_value() ? place->type : nullptr;
  }
  const Type* type = checkExpr(*unary.operand, nullptr);
  if (type == nullptr) {
    return nullptr;
  }
  if (type->kind == TypeKind::REF) {
    if (type->is_mutable) {
      error(unary.operand->span, "a mutable reference is not supported here");
      return nullptr;
    }
    if (type->target->kind == TypeKind::REF) {
      error(unary.operand->span, "only one shared reference layer is supported");
      return nullptr;
    }
    type = type->target;
  }
  if (unary.op == ast::Op::NEGATE) {
    if (!isInteger(type->kind) || !isSignedKind(type->kind)) {
      error(unary.operand->span, "unary minus requires a signed integer");
      return nullptr;
    }
    return type;
  }
  if (!isInteger(type->kind) && type->kind != TypeKind::BOOL) {
    error(unary.operand->span, "unsupported operand type");
  }
  return type;
}

const Type* Analyzer::checkBinary(const ast::BinaryExpr& binary) {
  const Type* left = checkExpr(*binary.left, nullptr);
  const Type* right = checkExpr(*binary.right, nullptr);
  if (left == nullptr || right == nullptr) {
    return nullptr;
  }
  if (binary.op == ast::Op::AND || binary.op == ast::Op::OR) {
    if (left->kind != TypeKind::BOOL) {
      error(binary.left->span, "a logical operand must be bool");
    }
    if (right->kind != TypeKind::BOOL) {
      error(binary.right->span, "a logical operand must be bool");
    }
    return make(TypeKind::BOOL);
  }
  if (binary.op == ast::Op::EQUAL || binary.op == ast::Op::NOT_EQUAL) {
    if (!sameType(left, right)) {
      error(binary.left->span, "equality requires both operands to have the same type");
      return make(TypeKind::BOOL);
    }
    if (!isPartialEq(left)) {
      error(binary.left->span, "this type does not support equality");
    }
    return make(TypeKind::BOOL);
  }
  if (isComparison(binary.op)) {
    const Type* l = left;
    const Type* r = right;
    while (l->kind == TypeKind::REF && r->kind == TypeKind::REF) {
      if (l->is_mutable && !r->is_mutable) {
        error(binary.left->span, "cannot compare a mutable reference with a shared one");
        return make(TypeKind::BOOL);
      }
      l = l->target;
      r = r->target;
    }
    if (l->kind == TypeKind::REF || r->kind == TypeKind::REF) {
      error(binary.left->span, "cannot compare a value with a reference");
      return make(TypeKind::BOOL);
    }
    if (!sameType(l, r) || (!isInteger(l->kind) && l->kind != TypeKind::BOOL)) {
      error(binary.left->span, "the operands do not have a comparable type");
    }
    return make(TypeKind::BOOL);
  }
  auto scalar = [&](const Type* type, const ast::Expr& expression) -> const Type* {
    if (type->kind != TypeKind::REF) {
      return type;
    }
    if (type->is_mutable) {
      error(expression.span, "a mutable reference is not supported as an operand");
      return nullptr;
    }
    if (type->target->kind == TypeKind::REF) {
      error(expression.span, "only one shared reference layer is supported");
      return nullptr;
    }
    return type->target;
  };
  const Type* l = scalar(left, *binary.left);
  const Type* r = scalar(right, *binary.right);
  if (l == nullptr || r == nullptr) {
    return nullptr;
  }
  if (isShift(binary.op)) {
    if (!isInteger(l->kind) || !isInteger(r->kind)) {
      error(binary.left->span, "a shift requires integer operands");
    }
    return l;
  }
  if (!sameType(l, r)) {
    error(binary.left->span, "the operands must have the same type");
    return l;
  }
  const bool is_bitwise = binary.op == ast::Op::BIT_AND || binary.op == ast::Op::BIT_OR
    || binary.op == ast::Op::BIT_XOR;
  if (is_bitwise) {
    if (!isInteger(l->kind) && l->kind != TypeKind::BOOL) {
      error(binary.left->span, "unsupported operand type");
    }
  } else if (!isInteger(l->kind)) {
    error(binary.left->span, "an arithmetic operand must be an integer");
  }
  return l;
}

const Type* Analyzer::checkAssign(const ast::AssignExpr& assign) {
  auto place = checkPlace(*assign.left);
  if (!place.has_value()) {
    checkExpr(*assign.left, nullptr);
    error(assign.left->span, "the assignment destination is not a place");
    checkExpr(*assign.right, nullptr);
    return make(TypeKind::UNIT);
  }
  requireMutable(assign.left->span, *place, "an assignment");
  const bool is_shift = assign.op == ast::Op::SHIFT_LEFT_ASSIGN || assign.op == ast::Op::SHIFT_RIGHT_ASSIGN;
  if (is_shift) {
    const Type* right = checkExpr(*assign.right, nullptr);
    if (right != nullptr) {
      const Type* value = right;
      if (value->kind == TypeKind::REF && !value->is_mutable && value->target->kind != TypeKind::REF) {
        value = value->target;
      }
      if (!isInteger(value->kind)) {
        error(assign.right->span, "a shift count must be an integer");
      }
    }
  } else {
    const Type* right = checkExpr(*assign.right, place->type);
    if (right != nullptr) {
      const Type* value = right;
      if (value->kind == TypeKind::REF && !value->is_mutable && value->target->kind != TypeKind::REF) {
        value = value->target;
      }
      coerce(*assign.right, value, place->type);
    }
  }
  return make(TypeKind::UNIT);
}

const Type* Analyzer::checkCast(const ast::CastExpr& cast) {
  const Type* source = checkExpr(*cast.expression, nullptr);
  const Type* target = resolveType(*cast.type);
  if (source == nullptr || target == nullptr) {
    return target;
  }
  if (isInteger(source->kind) && isInteger(target->kind)) {
    return target;
  }
  if (source->kind == TypeKind::BOOL && isInteger(target->kind)) {
    return target;
  }
  error(cast.expression->span, "unsupported cast");
  return target;
}

const Type* Analyzer::checkCall(const ast::CallExpr& call) {
  const auto* path = std::get_if<ast::PathExpr>(&call.callee->value);
  if (path == nullptr) {
    error(call.callee->span, "the callee must name a function");
    return nullptr;
  }
  const ast::Path& segments = path->path;
  if (segments.size() == 1) {
    const ast::PathSegment& segment = segments.front();
    if (!segment.arguments.empty()) {
      error(segment.span, "a function takes no type arguments");
      return nullptr;
    }
    if (lookupLocal(segment.name) != nullptr) {
      error(segment.span, "this name is not callable");
      return nullptr;
    }
    if (segment.name == "get_i32") {
      if (!call.arguments.empty()) {
        error(segment.span, "get_i32 takes no arguments");
      }
      return make(TypeKind::I32);
    }
    if (segment.name == "print_i32" || segment.name == "println_i32") {
      if (call.arguments.size() != 1) {
        error(segment.span, "this builtin takes exactly one argument");
        return make(TypeKind::UNIT);
      }
      const Type* type = checkExpr(*call.arguments.front(), make(TypeKind::I32));
      if (type != nullptr) {
        coerce(*call.arguments.front(), type, make(TypeKind::I32));
      }
      return make(TypeKind::UNIT);
    }
    auto found = functions_.find(segment.name);
    if (found == functions_.end()) {
      error(segment.span, "unresolved function name");
      return nullptr;
    }
    return checkOrdinaryCall(call, *found->second, nullptr);
  }
  if (segments.size() != 2) {
    error(segments.front().span, "unsupported call path");
    return nullptr;
  }
  const ast::PathSegment& owner = segments.front();
  const ast::PathSegment& member = segments.back();
  if (!member.arguments.empty()) {
    error(member.span, "a method path takes no type arguments");
  }
  if (owner.name == "Box" || owner.name == "Vec") {
    if (owner.arguments.size() != 1) {
      error(owner.span, "exactly one type argument is required");
      return nullptr;
    }
    const Type* element = resolveType(*owner.arguments.front());
    if (element == nullptr) {
      return nullptr;
    }
    const TypeKind kind = owner.name == "Box" ? TypeKind::BOX : TypeKind::VEC;
    const Type* container = containerType(kind, element);
    if (member.name == "new") {
      if (kind == TypeKind::BOX) {
        if (call.arguments.size() != 1) {
          error(member.span, "Box::new takes exactly one argument");
          return container;
        }
        const Type* type = checkExpr(*call.arguments.front(), element);
        if (type != nullptr) {
          coerce(*call.arguments.front(), type, element);
        }
      } else if (!call.arguments.empty()) {
        error(member.span, "Vec::new takes no arguments");
      }
      return container;
    }
    if (member.name == "clone") {
      if (call.arguments.size() != 1) {
        error(member.span, "clone takes exactly one argument");
        return container;
      }
      const Type* type = checkExpr(*call.arguments.front(), referenceType(container, false));
      if (type != nullptr) {
        coerce(*call.arguments.front(), type, referenceType(container, false));
      }
      if (!isClone(element)) {
        error(member.span, "the element type is not Clone");
      }
      return container;
    }
    if (member.name == "len" || member.name == "is_empty") {
      if (kind != TypeKind::VEC) {
        error(member.span, "unknown associated function");
        return nullptr;
      }
      const Type* expected = referenceType(container, false);
      if (call.arguments.size() != 1) {
        error(member.span, "this method takes exactly one argument");
      } else {
        const Type* type = checkExpr(*call.arguments.front(), expected);
        if (type != nullptr) {
          coerce(*call.arguments.front(), type, expected);
        }
      }
      return make(member.name == "len" ? TypeKind::USIZE : TypeKind::BOOL);
    }
    if (member.name == "push" || member.name == "remove") {
      if (kind != TypeKind::VEC) {
        error(member.span, "unknown associated function");
        return nullptr;
      }
      const Type* expected = referenceType(container, true);
      if (call.arguments.size() != 2) {
        error(member.span, "this method takes exactly two arguments");
      } else {
        const Type* receiver = checkExpr(*call.arguments.front(), expected);
        if (receiver != nullptr) {
          coerce(*call.arguments.front(), receiver, expected);
        }
        const Type* value = member.name == "push" ? element : make(TypeKind::USIZE);
        const Type* type = checkExpr(*call.arguments[1], value);
        if (type != nullptr) {
          coerce(*call.arguments[1], type, value);
        }
      }
      return member.name == "push" ? make(TypeKind::UNIT) : element;
    }
    error(member.span, "unknown associated function");
    return nullptr;
  }
  const StructInfo* info = lookupStruct(owner);
  if (info == nullptr) {
    error(owner.span, "unresolved type name");
    return nullptr;
  }
  auto found = info->functions.find(member.name);
  if (found == info->functions.end()) {
    if (member.name == "clone" && info->has_clone) {
      const Type* expected = referenceType(structType(*info), false);
      if (call.arguments.size() != 1) {
        error(member.span, "clone takes exactly one argument");
        return expected->target;
      }
      const Type* type = checkExpr(*call.arguments.front(), expected);
      if (type != nullptr) {
        coerce(*call.arguments.front(), type, expected);
      }
      return expected->target;
    }
    error(member.span, "no such associated function");
    return nullptr;
  }
  return checkOrdinaryCall(call, *found->second.declaration, structType(*info));
}

const Type* Analyzer::checkOrdinaryCall(const ast::CallExpr& call,
  const ast::FunctionItem& function, const Type* receiver_type) {
  const StructInfo* saved = self_struct_;
  if (receiver_type != nullptr && receiver_type->kind == TypeKind::STRUCT) {
    const StructInfo* owner = structInfo(receiver_type->declaration);
    if (owner != nullptr) {
      self_struct_ = owner;
    }
  }
  std::vector<const Type*> parameters;
  if (receiver_type != nullptr && function.receiver.has_value()) {
    parameters.push_back(function.receiver->is_reference
      ? referenceType(receiver_type, function.receiver->is_mutable) : receiver_type);
  }
  for (const auto& parameter : function.parameters) {
    parameters.push_back(resolveType(*parameter.type));
  }
  if (call.arguments.size() != parameters.size()) {
    error(call.callee->span, "wrong number of arguments");
  } else {
    for (std::size_t index = 0; index < parameters.size(); ++index) {
      const Type* type = checkExpr(*call.arguments[index], parameters[index]);
      if (type != nullptr) {
        coerce(*call.arguments[index], type, parameters[index]);
      }
    }
  }
  const Type* result = make(TypeKind::UNIT);
  if (function.return_type != nullptr) {
    result = resolveType(*function.return_type);
  }
  self_struct_ = saved;
  return result;
}

const Type* Analyzer::checkMethodCall(const ast::MethodCallExpr& call) {
  const Type* receiver = checkExpr(*call.receiver, nullptr);
  if (receiver == nullptr) {
    return nullptr;
  }
  if (!call.method.arguments.empty()) {
    error(call.method.span, "a method path takes no type arguments");
  }
  const std::string& name = call.method.name;
  const Type* current = receiver;
  for (std::size_t derefs = 0; current != nullptr; ++derefs) {
    for (int variant = 0; variant < 3; ++variant) {
      const Type* candidate = variant == 0 ? current : referenceType(current, variant == 2);
      const Type* shape = candidate->kind == TypeKind::REF ? candidate->target : candidate;
      const bool is_mutable_candidate = candidate->kind == TypeKind::REF && candidate->is_mutable;
      const Type* result = nullptr;
      bool matched = false;
      const bool wants_clone = name == "clone" && !is_mutable_candidate;
      if (wants_clone && shape->kind == TypeKind::REF) {
        matched = true;
        result = shape;
      } else if (wants_clone && (isInteger(shape->kind) || shape->kind == TypeKind::BOOL)) {
        matched = true;
        result = shape;
      } else if (wants_clone && shape->kind == TypeKind::ARRAY) {
        matched = true;
        result = shape;
        if (!isClone(shape->target)) {
          error(call.method.span, "the element type is not Clone");
        }
      } else if (shape->kind == TypeKind::VEC) {
        if (name == "len" || name == "is_empty") {
          matched = call.arguments.empty();
          if (!matched) {
            error(call.method.span, "this method takes no arguments");
          }
          result = make(name == "len" ? TypeKind::USIZE : TypeKind::BOOL);
        } else if (name == "clone" && !is_mutable_candidate) {
          matched = true;
          result = containerType(TypeKind::VEC, shape->target);
          if (!isClone(shape->target)) {
            error(call.method.span, "the element type is not Clone");
          }
        } else if ((name == "push" || name == "remove") && is_mutable_candidate) {
          matched = true;
          if (call.arguments.size() != 1) {
            error(call.method.span, "this method takes exactly one argument");
          } else {
            const Type* expected = name == "push" ? shape->target : make(TypeKind::USIZE);
            const Type* type = checkExpr(*call.arguments.front(), expected);
            if (type != nullptr) {
              coerce(*call.arguments.front(), type, expected);
            }
          }
          result = name == "push" ? make(TypeKind::UNIT) : shape->target;
        }
      } else if (shape->kind == TypeKind::BOX) {
        if (name == "clone" && !is_mutable_candidate) {
          matched = true;
          result = containerType(TypeKind::BOX, shape->target);
          if (!isClone(shape->target)) {
            error(call.method.span, "the element type is not Clone");
          }
        }
      } else if (shape->kind == TypeKind::ARRAY) {
        if (name == "len") {
          matched = true;
          if (!call.arguments.empty()) {
            error(call.method.span, "this method takes no arguments");
          }
          result = make(TypeKind::USIZE);
        }
      } else if (shape->kind == TypeKind::STRUCT) {
        const StructInfo* info = structInfo(shape->declaration);
        if (wants_clone && info != nullptr && info->has_clone) {
          matched = true;
          result = shape;
        }
      }
      if (!matched) {
        const FunctionInfo* inherent = findInherent(candidate, name);
        if (inherent == nullptr) {
          continue;
        }
        const ast::FunctionItem& method = *inherent->declaration;
        const StructInfo* saved = self_struct_;
        const StructInfo* owner = structInfo(shape->declaration);
        if (owner != nullptr) {
          self_struct_ = owner;
        }
        if (method.receiver->is_mutable) {
          auto place = checkPlace(*call.receiver);
          if (place.has_value()) {
            auto advanced = advancePlace(place, derefs);
            bool is_blocked = false;
            if (!advanced.has_value()) {
              is_blocked = true;
            } else if (variant == 2) {
              is_blocked = !advanced->is_mutable;
            } else {
              is_blocked = advanced->is_borrowed_shared;
            }
            if (is_blocked) {
              error(call.method.span, "this method requires a mutable receiver");
            }
          }
        }
        if (call.arguments.size() != method.parameters.size()) {
          error(call.method.span, "wrong number of arguments");
        } else {
          for (std::size_t index = 0; index < method.parameters.size(); ++index) {
            const Type* expected = resolveType(*method.parameters[index].type);
            const Type* type = checkExpr(*call.arguments[index], expected);
            if (type != nullptr) {
              coerce(*call.arguments[index], type, expected);
            }
          }
        }
        const Type* result = make(TypeKind::UNIT);
        if (method.return_type != nullptr) {
          result = resolveType(*method.return_type);
        }
        self_struct_ = saved;
        return result;
      }
      if (is_mutable_candidate) {
        auto place = checkPlace(*call.receiver);
        if (place.has_value()) {
          auto advanced = advancePlace(place, derefs);
          bool is_blocked = false;
          if (!advanced.has_value()) {
            is_blocked = true;
          } else if (variant == 2) {
            is_blocked = !advanced->is_mutable;
          } else {
            is_blocked = advanced->is_borrowed_shared;
          }
          if (is_blocked) {
            error(call.method.span, "this call requires a mutable receiver");
          }
        }
      }
      return result;
    }
    const Type* next = derefOnce(current);
    if (next == nullptr) {
      break;
    }
    current = next;
  }
  error(call.method.span, "no matching method");
  return nullptr;
}

const Type* Analyzer::checkField(const ast::FieldExpr& field) {
  auto place = placeOfField(field);
  return place.has_value() ? place->type : nullptr;
}

const Type* Analyzer::checkIndex(const ast::IndexExpr& index) {
  auto place = placeOfIndex(index);
  return place.has_value() ? place->type : nullptr;
}

const Type* Analyzer::checkStructExpr(const ast::StructExpr& literal) {
  if (literal.path.size() != 1) {
    error(literal.path.front().span, "unsupported struct path");
    return nullptr;
  }
  const StructInfo* info = lookupStruct(literal.path.front());
  if (info == nullptr) {
    error(literal.path.front().span, "unresolved struct name");
    return nullptr;
  }
  std::set<std::string> seen;
  for (const auto& field : literal.fields) {
    if (!seen.insert(field.name).second) {
      error(field.span, "duplicate initializer field");
      continue;
    }
    std::size_t index = info->declaration->fields.size();
    for (std::size_t i = 0; i < info->declaration->fields.size(); ++i) {
      if (info->declaration->fields[i].name == field.name) {
        index = i;
      }
    }
    if (index == info->declaration->fields.size()) {
      error(field.span, "unknown initializer field");
      checkExpr(*field.value, nullptr);
      continue;
    }
    const Type* expected = info->field_types[index];
    const Type* value = checkExpr(*field.value, expected);
    if (value != nullptr && expected != nullptr) {
      coerce(*field.value, value, expected);
    }
  }
  for (const auto& declared : info->declaration->fields) {
    if (seen.count(declared.name) == 0) {
      error(literal.path.front().span, "missing initializer field");
    }
  }
  return structType(*info);
}

const Type* Analyzer::checkArray(const ast::ArrayExpr& array, const Type* expected) {
  const Type* element = expected != nullptr && expected->kind == TypeKind::ARRAY ? expected->target : nullptr;
  if (element != nullptr && expected->length != array.elements.size()) {
    error(array.span, "the array length does not match");
  }
  std::vector<const Type*> types;
  for (const auto& item : array.elements) {
    const Type* type = checkExpr(*item, element);
    if (element != nullptr && type != nullptr) {
      coerce(*item, type, element);
    }
    types.push_back(type);
  }
  if (element != nullptr) {
    return expected;
  }
  return arrayType(lubOf(types), array.elements.size());
}

const Type* Analyzer::checkRepeat(const ast::RepeatExpr& repeat, const Type* expected) {
  const Type* element = expected != nullptr && expected->kind == TypeKind::ARRAY ? expected->target : nullptr;
  const Type* value = checkExpr(*repeat.element, element);
  auto length = evalConst(*repeat.length, make(TypeKind::USIZE));
  if (!length.has_value()) {
    return nullptr;
  }
  if (length->type->kind != TypeKind::USIZE) {
    error(repeat.length->span, "a repeat count must have type usize");
    return nullptr;
  }
  if (length->value > 1 && value != nullptr && !isCopy(value)) {
    error(repeat.element->span, "repeating more than once requires a Copy element");
  }
  return arrayType(value, static_cast<std::size_t>(length->value));
}

const Type* Analyzer::checkIf(const ast::IfExpr& branch, const Type* expected) {
  expectBool(*branch.condition, "an if condition");
  if (branch.else_branch == nullptr) {
    const Type* unit = make(TypeKind::UNIT);
    const Type* then_type = checkExpr(*branch.then_branch, unit);
    if (then_type != nullptr && adjustTo(then_type, unit) == nullptr) {
      error(branch.then_branch->span, "an if without else requires a unit body");
    }
    return unit;
  }
  const Type* then_type = checkExpr(*branch.then_branch, expected);
  const Type* else_type = checkExpr(*branch.else_branch, expected);
  if (expected != nullptr) {
    if (then_type != nullptr) {
      coerce(*branch.then_branch, then_type, expected);
    }
    if (else_type != nullptr) {
      coerce(*branch.else_branch, else_type, expected);
    }
    return expected;
  }
  return lubOf({then_type, else_type});
}

const Type* Analyzer::checkLoop(const ast::LoopExpr& loop, const Type* expected) {
  loops_.push_back(LoopContext{true, expected, {}, false});
  const ast::BlockExpr* body = asBlock(*loop.body);
  if (body != nullptr) {
    const Type* unit = make(TypeKind::UNIT);
    const Type* type = checkBlock(*body, unit);
    if (type != nullptr && adjustTo(type, unit) == nullptr) {
      error(loop.body->span, "a loop body must have unit type");
    }
  }
  LoopContext context = loops_.back();
  loops_.pop_back();
  if (expected != nullptr) {
    return expected;
  }
  if (!context.has_break) {
    return make(TypeKind::NEVER);
  }
  return lubOf(context.break_types);
}

const Type* Analyzer::checkWhile(const ast::WhileExpr& loop) {
  condition_barriers_.push_back(loops_.size());
  expectBool(*loop.condition, "a while condition");
  condition_barriers_.pop_back();
  loops_.push_back(LoopContext{false, nullptr, {}, false});
  const ast::BlockExpr* body = asBlock(*loop.body);
  if (body != nullptr) {
    const Type* unit = make(TypeKind::UNIT);
    const Type* type = checkBlock(*body, unit);
    if (type != nullptr && adjustTo(type, unit) == nullptr) {
      error(loop.body->span, "a while body must have unit type");
    }
  }
  loops_.pop_back();
  return make(TypeKind::UNIT);
}

const Type* Analyzer::checkBreak(const ast::BreakExpr& jump) {
  if (loops_.empty()) {
    error(jump.span, "break outside of a loop");
    if (jump.value != nullptr) {
      checkExpr(*jump.value, nullptr);
    }
    return make(TypeKind::NEVER);
  }
  if (!condition_barriers_.empty() && loops_.size() <= condition_barriers_.back()) {
    error(jump.span, "this break targets a loop outside the condition");
  }
  LoopContext& context = loops_.back();
  if (jump.value != nullptr) {
    if (!context.accepts_value) {
      error(jump.span, "a while cannot accept a break value");
    }
    const Type* type = checkExpr(*jump.value, context.expected);
    if (type != nullptr && context.expected != nullptr) {
      coerce(*jump.value, type, context.expected);
    }
    context.has_break = true;
    context.break_types.push_back(type);
  } else {
    context.has_break = true;
    context.break_types.push_back(make(TypeKind::UNIT));
  }
  return make(TypeKind::NEVER);
}

const Type* Analyzer::checkContinue(const ast::ContinueExpr& jump) {
  if (loops_.empty()) {
    error(jump.span, "continue outside of a loop");
    return make(TypeKind::NEVER);
  }
  if (!condition_barriers_.empty() && loops_.size() <= condition_barriers_.back()) {
    error(jump.span, "this continue targets a loop outside the condition");
  }
  return make(TypeKind::NEVER);
}

const Type* Analyzer::checkReturn(const ast::ReturnExpr& jump) {
  if (jump.value != nullptr) {
    const Type* type = checkExpr(*jump.value, return_type_);
    if (type != nullptr && return_type_ != nullptr) {
      coerce(*jump.value, type, return_type_);
    }
  } else if (return_type_ != nullptr && return_type_->kind != TypeKind::UNIT) {
    error(jump.span, "a bare return has unit type");
  }
  return make(TypeKind::NEVER);
}

std::optional<Place> Analyzer::placeOfPath(const ast::PathExpr& path) {
  if (path.path.size() != 1) {
    return std::nullopt;
  }
  LocalVar* local = lookupLocal(path.path.front().name);
  if (local == nullptr) {
    return std::nullopt;
  }
  return Place{local->type, local->is_mutable, false};
}

std::optional<Place> Analyzer::placeOfUnary(const ast::UnaryExpr& unary) {
  if (unary.op != ast::Op::DEREFERENCE) {
    return std::nullopt;
  }
  auto base = checkPlace(*unary.operand);
  if (!base.has_value()) {
    const Type* type = checkExpr(*unary.operand, nullptr);
    if (type == nullptr) {
      return std::nullopt;
    }
    if (type->kind == TypeKind::REF) {
      return narrowPlace(Place{type->target, type->is_mutable, !type->is_mutable});
    }
    if (type->kind == TypeKind::BOX) {
      return narrowPlace(Place{type->target, true, false});
    }
    error(unary.operand->span, "this value cannot be dereferenced");
    return std::nullopt;
  }
  const Type* type = base->type;
  if (type == nullptr) {
    return std::nullopt;
  }
  if (type->kind == TypeKind::REF) {
    const bool shared = base->is_borrowed_shared || !type->is_mutable;
    return narrowPlace(Place{type->target, !shared, shared});
  }
  if (type->kind == TypeKind::BOX) {
    return narrowPlace(Place{type->target, base->is_mutable, base->is_borrowed_shared});
  }
  error(unary.operand->span, "this value cannot be dereferenced");
  return std::nullopt;
}

std::optional<Place> Analyzer::placeOfField(const ast::FieldExpr& field) {
  auto base = checkPlace(*field.object);
  const Type* type = nullptr;
  bool is_mutable = true;
  bool shared = false;
  if (base.has_value()) {
    type = base->type;
    is_mutable = base->is_mutable;
    shared = base->is_borrowed_shared;
  } else {
    type = checkExpr(*field.object, nullptr);
  }
  if (type == nullptr) {
    return std::nullopt;
  }
  while (type->kind == TypeKind::REF || type->kind == TypeKind::BOX) {
    if (type->kind == TypeKind::REF) {
      if (!type->is_mutable) {
        shared = true;
      }
      is_mutable = !shared;
    } else {
      is_mutable = is_mutable && !shared;
    }
    type = type->target;
  }
  const Type* member = fieldOf(type, field.name, field.object->span);
  if (member == nullptr) {
    return std::nullopt;
  }
  return narrowPlace(Place{member, is_mutable, shared});
}

std::optional<Place> Analyzer::placeOfIndex(const ast::IndexExpr& index) {
  auto base = checkPlace(*index.object);
  const Type* type = nullptr;
  bool is_mutable = true;
  bool shared = false;
  if (base.has_value()) {
    type = base->type;
    is_mutable = base->is_mutable;
    shared = base->is_borrowed_shared;
  } else {
    type = checkExpr(*index.object, nullptr);
  }
  if (type == nullptr) {
    return std::nullopt;
  }
  while (type->kind == TypeKind::REF || type->kind == TypeKind::BOX) {
    if (type->kind == TypeKind::REF) {
      if (!type->is_mutable) {
        shared = true;
      }
      is_mutable = !shared;
    } else {
      is_mutable = is_mutable && !shared;
    }
    type = type->target;
  }
  if (type->kind != TypeKind::ARRAY && type->kind != TypeKind::VEC) {
    error(index.object->span, "this type cannot be indexed");
    checkExpr(*index.index, nullptr);
    return std::nullopt;
  }
  const Type* key = checkExpr(*index.index, make(TypeKind::USIZE));
  if (key != nullptr && key->kind != TypeKind::USIZE) {
    error(index.index->span, "an index must have type usize");
  }
  if (type->kind == TypeKind::VEC) {
    const bool vector_mutable = is_mutable && !shared;
    return narrowPlace(Place{type->target, vector_mutable, shared || !vector_mutable});
  }
  return narrowPlace(Place{type->target, is_mutable, shared});
}

std::optional<Place> Analyzer::checkPlace(const ast::Expr& expression) {
  return std::visit([&](const auto& value) -> std::optional<Place> {
    using Node = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<Node, ast::PathExpr>) {
      return placeOfPath(value);
    } else if constexpr (std::is_same_v<Node, ast::FieldExpr>) {
      return placeOfField(value);
    } else if constexpr (std::is_same_v<Node, ast::IndexExpr>) {
      return placeOfIndex(value);
    } else if constexpr (std::is_same_v<Node, ast::UnaryExpr>) {
      return placeOfUnary(value);
    } else {
      return std::nullopt;
    }
  }, expression.value);
}

const Type* Analyzer::checkExpr(const ast::Expr& expression, const Type* expected) {
  return std::visit([&](const auto& value) -> const Type* {
    using Node = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<Node, ast::IntegerExpr>) {
      return checkIntegerLiteral(value, expected);
    } else if constexpr (std::is_same_v<Node, ast::BoolExpr>) {
      return make(TypeKind::BOOL);
    } else if constexpr (std::is_same_v<Node, ast::UnitExpr>) {
      return make(TypeKind::UNIT);
    } else if constexpr (std::is_same_v<Node, ast::PathExpr>) {
      return checkPathExpr(value);
    } else if constexpr (std::is_same_v<Node, ast::UnaryExpr>) {
      return checkUnary(value);
    } else if constexpr (std::is_same_v<Node, ast::BinaryExpr>) {
      return checkBinary(value);
    } else if constexpr (std::is_same_v<Node, ast::AssignExpr>) {
      return checkAssign(value);
    } else if constexpr (std::is_same_v<Node, ast::CastExpr>) {
      return checkCast(value);
    } else if constexpr (std::is_same_v<Node, ast::CallExpr>) {
      return checkCall(value);
    } else if constexpr (std::is_same_v<Node, ast::MethodCallExpr>) {
      return checkMethodCall(value);
    } else if constexpr (std::is_same_v<Node, ast::FieldExpr>) {
      return checkField(value);
    } else if constexpr (std::is_same_v<Node, ast::IndexExpr>) {
      return checkIndex(value);
    } else if constexpr (std::is_same_v<Node, ast::StructExpr>) {
      return checkStructExpr(value);
    } else if constexpr (std::is_same_v<Node, ast::ArrayExpr>) {
      return checkArray(value, expected);
    } else if constexpr (std::is_same_v<Node, ast::RepeatExpr>) {
      return checkRepeat(value, expected);
    } else if constexpr (std::is_same_v<Node, ast::BlockExpr>) {
      return checkBlock(value, expected);
    } else if constexpr (std::is_same_v<Node, ast::IfExpr>) {
      return checkIf(value, expected);
    } else if constexpr (std::is_same_v<Node, ast::LoopExpr>) {
      return checkLoop(value, expected);
    } else if constexpr (std::is_same_v<Node, ast::WhileExpr>) {
      return checkWhile(value);
    } else if constexpr (std::is_same_v<Node, ast::BreakExpr>) {
      return checkBreak(value);
    } else if constexpr (std::is_same_v<Node, ast::ContinueExpr>) {
      return checkContinue(value);
    } else {
      return checkReturn(value);
    }
  }, expression.value);
}
}
