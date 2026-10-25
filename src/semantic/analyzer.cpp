#include "analyzer.h"
#include <set>
#include <string>
#include <cctype>
#include <variant>
#include <algorithm>
#include <type_traits>

namespace rx::semantic {
namespace {
// Rx 语言本身提供的内置函数，用户定义的函数和变量不能与它们重名
constexpr const char* PROTECTED_VALUE_NAMES[] = {"get_i32", "print_i32", "println_i32"};

bool isSignedKind(TypeKind kind) {
  return kind == TypeKind::I32 || kind == TypeKind::ISIZE;
}

std::int64_t parseMagnitude(const std::string& digits, unsigned base) {
  std::uint64_t value = 0;
  for (char character : digits) {
    std::uint64_t digit = 0;
    if (character >= '0' && character <= '9') {
      digit = static_cast<std::uint64_t>(character - '0');
    } else if (character >= 'a' && character <= 'f') {
      digit = static_cast<std::uint64_t>(character - 'a' + 10);
    } else if (character >= 'A' && character <= 'F') {
      digit = static_cast<std::uint64_t>(character - 'A' + 10);
    }
    if (value > (UINT64_MAX / 16)) {
      return static_cast<std::int64_t>(value);
    }
    value = value * base + digit;
  }
  return static_cast<std::int64_t>(value);
}
}

void analyze(const ast::Crate& crate, const std::string& file, std::vector<Diagnostic>& diagnostics) {
  Analyzer analyzer;
  analyzer.run(crate, file, diagnostics);
}

void Analyzer::run(const ast::Crate& crate, const std::string& file, std::vector<Diagnostic>& diagnostics) {
  file_ = file;
  diagnostics_ = &diagnostics;
  collectStructs(crate);
  collectValues(crate);
  collectImpls(crate);
  checkDerives();
  checkLayout();
  checkEntry(crate);
  checkSignatures(crate);
  for (const auto& item : crate.items) {
    if (const auto* function = std::get_if<ast::FunctionItem>(&item->value)) {
      checkFunction(*function);
    } else if (const auto* impl = std::get_if<ast::ImplItem>(&item->value)) {
      self_struct_ = implTarget(*impl);
      if (self_struct_ == nullptr) {
        continue;
      }
      for (const auto& member : impl->items) {
        if (const auto* method = std::get_if<ast::FunctionItem>(&member->value)) {
          checkFunction(*method);
        }
      }
      self_struct_ = nullptr;
    }
  }
}

void Analyzer::error(const ast::Span& span, const std::string& message) {
  diagnostics_->push_back({file_, span.line, span.column, "semantic", message});
}

const Type* Analyzer::make(TypeKind kind) {
  arena_.push_back(Type{kind});
  return &arena_.back();
}

const Type* Analyzer::integerType(const std::string& name) {
  if (name == "i32") {
    return make(TypeKind::I32);
  }
  if (name == "u32") {
    return make(TypeKind::U32);
  }
  if (name == "isize") {
    return make(TypeKind::ISIZE);
  }
  if (name == "usize") {
    return make(TypeKind::USIZE);
  }
  return nullptr;
}

const Type* Analyzer::arrayType(const Type* element, std::size_t length) {
  arena_.push_back(Type{TypeKind::ARRAY});
  Type* type = &arena_.back();
  type->target = element;
  type->length = length;
  return type;
}

const Type* Analyzer::referenceType(const Type* target, bool is_mutable) {
  arena_.push_back(Type{TypeKind::REF});
  Type* type = &arena_.back();
  type->target = target;
  type->is_mutable = is_mutable;
  return type;
}

const Type* Analyzer::containerType(TypeKind kind, const Type* element) {
  arena_.push_back(Type{kind});
  Type* type = &arena_.back();
  type->target = element;
  return type;
}

const Type* Analyzer::structType(const StructInfo& info) {
  arena_.push_back(Type{TypeKind::STRUCT});
  Type* type = &arena_.back();
  type->declaration = info.declaration;
  return type;
}

const StructInfo* Analyzer::structInfo(const ast::StructItem* declaration) {
  auto found = structs_.find(declaration->name);
  return found == structs_.end() ? nullptr : &found->second;
}

const StructInfo* Analyzer::implTarget(const ast::ImplItem& impl) {
  const auto* path = std::get_if<ast::PathType>(&impl.type->value);
  if (path == nullptr || path->path.size() != 1) {
    return nullptr;
  }
  auto found = structs_.find(path->path.front().name);
  return found == structs_.end() ? nullptr : &found->second;
}

const StructInfo* Analyzer::lookupStruct(const ast::PathSegment& segment) {
  if (segment.name == "Self") {
    return self_struct_;
  }
  auto found = structs_.find(segment.name);
  return found == structs_.end() ? nullptr : &found->second;
}

const Type* Analyzer::resolvePathType(const ast::Path& path) {
  if (path.size() != 1) {
    error(path.front().span, "unsupported type path");
    return nullptr;
  }
  const ast::PathSegment& segment = path.front();
  if (!segment.arguments.empty()) {
    if (segment.name != "Box" && segment.name != "Vec") {
      error(segment.span, "this type takes no type arguments");
      return nullptr;
    }
    if (segment.arguments.size() != 1) {
      error(segment.span, "exactly one type argument is required");
      return nullptr;
    }
    const Type* element = resolveType(*segment.arguments.front());
    if (element == nullptr) {
      return nullptr;
    }
    return containerType(segment.name == "Box" ? TypeKind::BOX : TypeKind::VEC, element);
  }
  if (const Type* primitive = integerType(segment.name)) {
    return primitive;
  }
  if (segment.name == "bool") {
    return make(TypeKind::BOOL);
  }
  if (segment.name == "Box" || segment.name == "Vec") {
    error(segment.span, "a type argument is required");
    return nullptr;
  }
  const StructInfo* info = lookupStruct(segment);
  if (info == nullptr) {
    error(segment.span, "unresolved type name");
    return nullptr;
  }
  return structType(*info);
}

const Type* Analyzer::resolveType(const ast::Type& type) {
  return std::visit([&](const auto& value) -> const Type* {
    using Node = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<Node, ast::PathType>) {
      return resolvePathType(value.path);
    } else if constexpr (std::is_same_v<Node, ast::UnitType>) {
      return make(TypeKind::UNIT);
    } else if constexpr (std::is_same_v<Node, ast::RefType>) {
      const Type* inner = resolveType(*value.inner);
      return inner == nullptr ? nullptr : referenceType(inner, value.is_mutable);
    } else {
      const Type* element = resolveType(*value.element);
      if (element == nullptr) {
        return nullptr;
      }
      const Type* usize = make(TypeKind::USIZE);
      auto length = evalConst(*value.length, usize);
      if (!length.has_value()) {
        return nullptr;
      }
      if (length->type->kind != TypeKind::USIZE) {
        error(value.length->span, "an array length must have type usize");
        return nullptr;
      }
      return arrayType(element, static_cast<std::size_t>(length->value));
    }
  }, type.value);
}

void Analyzer::collectStructs(const ast::Crate& crate) {
  for (const auto& item : crate.items) {
    if (const auto* structure = std::get_if<ast::StructItem>(&item->value)) {
      declareStruct(*structure);
    }
  }
}

void Analyzer::declareStruct(const ast::StructItem& item) {
  if (item.name == "i32") {
    error(item.span, "a struct cannot redeclare a builtin type name");
    return;
  }
  if (structs_.count(item.name)) {
    error(item.span, "duplicate struct");
    return;
  }
  std::set<std::string> fields;
  for (const auto& field : item.fields) {
    if (!fields.insert(field.name).second) {
      error(field.span, "duplicate field");
    }
  }
  StructInfo info;
  info.span = item.span;
  info.declaration = &item;
  std::set<std::string> derives;
  for (const auto& name : item.derives) {
    if (!derives.insert(name).second) {
      error(item.span, "repeated derive entry");
    }
  }
  info.has_copy = derives.count("Copy") != 0;
  info.has_clone = derives.count("Clone") != 0;
  info.has_partial_eq = derives.count("PartialEq") != 0;
  info.has_eq = derives.count("Eq") != 0;
  structs_.emplace(item.name, info);
}

void Analyzer::collectValues(const ast::Crate& crate) {
  for (const auto& item : crate.items) {
    if (const auto* function = std::get_if<ast::FunctionItem>(&item->value)) {
      declareFunction(*function);
    } else if (const auto* constant = std::get_if<ast::ConstItem>(&item->value)) {
      declareConstant(*constant);
    }
  }
}

void Analyzer::declareFunction(const ast::FunctionItem& item) {
  for (const char* name : PROTECTED_VALUE_NAMES) {
    if (item.name == name) {
      error(item.span, "a function cannot use a builtin name");
      return;
    }
  }
  if (item.receiver.has_value()) {
    error(item.span, "a top-level function cannot have a receiver");
    return;
  }
  if (functions_.count(item.name) || constants_.count(item.name)) {
    error(item.span, "duplicate top-level name");
    return;
  }
  functions_.emplace(item.name, &item);
}

void Analyzer::declareConstant(const ast::ConstItem& item) {
  for (const char* name : PROTECTED_VALUE_NAMES) {
    if (item.name == name) {
      error(item.span, "a constant cannot use a builtin name");
      return;
    }
  }
  if (functions_.count(item.name) || constants_.count(item.name)) {
    error(item.span, "duplicate top-level name");
    return;
  }
  constants_.emplace(item.name, &item);
}

void Analyzer::collectImpls(const ast::Crate& crate) {
  for (const auto& item : crate.items) {
    const auto* impl = std::get_if<ast::ImplItem>(&item->value);
    if (impl == nullptr) {
      continue;
    }
    const auto* path = std::get_if<ast::PathType>(&impl->type->value);
    if (path == nullptr || path->path.size() != 1) {
      error(impl->span, "an impl target must be a user-defined struct");
      continue;
    }
    auto found = structs_.find(path->path.front().name);
    if (found == structs_.end()) {
      error(impl->span, "an impl target must be a user-defined struct");
      continue;
    }
    StructInfo& info = found->second;
    for (const auto& member : impl->items) {
      if (const auto* function = std::get_if<ast::FunctionItem>(&member->value)) {
        if (info.functions.count(function->name) || info.constants.count(function->name)) {
          error(function->span, "duplicate associated item");
          continue;
        }
        info.functions.emplace(function->name, FunctionInfo{function});
      } else if (const auto* constant = std::get_if<ast::ConstItem>(&member->value)) {
        if (info.functions.count(constant->name) || info.constants.count(constant->name)) {
          error(constant->span, "duplicate associated item");
          continue;
        }
        info.constants.emplace(constant->name, constant);
      }
    }
  }
}

void Analyzer::checkDerives() {
  for (auto& entry : structs_) {
    StructInfo& info = entry.second;
    if (info.has_copy && !info.has_clone) {
      error(info.span, "Copy requires an explicit Clone derive");
    }
    if (info.has_eq && !info.has_partial_eq) {
      error(info.span, "Eq requires an explicit PartialEq derive");
    }
  }
  for (auto& entry : structs_) {
    StructInfo& info = entry.second;
    self_struct_ = &info;
    for (const auto& field : info.declaration->fields) {
      const Type* type = resolveType(*field.type);
      info.field_types.push_back(type);
      if (type == nullptr) {
        continue;
      }
      if (info.has_copy && !isCopy(type)) {
        error(field.span, "this field cannot be Copy");
      }
      if (info.has_clone && !isClone(type)) {
        error(field.span, "this field cannot be Clone");
      }
      if (info.has_partial_eq && !isPartialEq(type)) {
        error(field.span, "this field cannot be PartialEq");
      }
      if (info.has_eq && !isEq(type)) {
        error(field.span, "this field cannot be Eq");
      }
    }
    self_struct_ = nullptr;
  }
}

void Analyzer::checkLayout() {
  for (auto& entry : structs_) {
    std::vector<const StructInfo*> stack;
    hasInlineCycle(entry.second, stack);
  }
}

bool Analyzer::hasInlineCycle(const StructInfo& info, std::vector<const StructInfo*>& stack) {
  if (std::find(stack.begin(), stack.end(), &info) != stack.end()) {
    error(info.span, "this type contains itself without indirection");
    return true;
  }
  stack.push_back(&info);
  self_struct_ = &info;
  bool cyclic = false;
  for (const Type* field_type : info.field_types) {
    const Type* type = field_type;
    while (type != nullptr && type->kind == TypeKind::ARRAY) {
      type = type->target;
    }
    if (type == nullptr || type->kind != TypeKind::STRUCT) {
      continue;
    }
    const StructInfo* nested = structInfo(type->declaration);
    if (nested != nullptr && hasInlineCycle(*nested, stack)) {
      cyclic = true;
    }
  }
  self_struct_ = nullptr;
  stack.pop_back();
  return cyclic;
}

void Analyzer::checkEntry(const ast::Crate& crate) {
  auto found = functions_.find("main");
  if (found == functions_.end()) {
    error(crate.span, "the program has no top-level main function");
    return;
  }
  const ast::FunctionItem& main = *found->second;
  if (!main.parameters.empty()) {
    error(main.span, "main cannot have value parameters");
  }
  if (main.has_generics) {
    error(main.span, "main cannot have generic parameters");
  }
  if (main.return_type != nullptr) {
    const Type* type = resolveType(*main.return_type);
    if (type != nullptr && type->kind != TypeKind::UNIT) {
      error(main.span, "main must return unit");
    }
  }
}

void Analyzer::checkSignatures(const ast::Crate& crate) {
  for (const auto& item : crate.items) {
    if (const auto* function = std::get_if<ast::FunctionItem>(&item->value)) {
      checkSignature(*function);
    } else if (const auto* constant = std::get_if<ast::ConstItem>(&item->value)) {
      constantValue(*constant);
    } else if (const auto* impl = std::get_if<ast::ImplItem>(&item->value)) {
      self_struct_ = implTarget(*impl);
      for (const auto& member : impl->items) {
        if (const auto* function = std::get_if<ast::FunctionItem>(&member->value)) {
          checkSignature(*function);
        } else if (const auto* constant = std::get_if<ast::ConstItem>(&member->value)) {
          constantValue(*constant);
        }
      }
      self_struct_ = nullptr;
    }
  }
}

void Analyzer::checkSignature(const ast::FunctionItem& item) {
  std::set<std::string> names;
  for (const auto& parameter : item.parameters) {
    if (!names.insert(parameter.name).second) {
      error(parameter.span, "duplicate parameter name");
    }
    resolveType(*parameter.type);
  }
  if (item.return_type != nullptr) {
    resolveType(*item.return_type);
  }
}

bool Analyzer::isCopy(const Type* type) {
  if (type == nullptr) {
    return false;
  }
  switch (type->kind) {
    case TypeKind::I32:
    case TypeKind::U32:
    case TypeKind::ISIZE:
    case TypeKind::USIZE:
    case TypeKind::BOOL:
      return true;
    case TypeKind::REF:
      return !type->is_mutable;
    case TypeKind::ARRAY:
      return isCopy(type->target);
    case TypeKind::STRUCT: {
      const StructInfo* info = structInfo(type->declaration);
      return info != nullptr && info->has_copy;
    }
    default:
      return false;
  }
}

bool Analyzer::isClone(const Type* type) {
  if (type == nullptr) {
    return false;
  }
  switch (type->kind) {
    case TypeKind::I32:
    case TypeKind::U32:
    case TypeKind::ISIZE:
    case TypeKind::USIZE:
    case TypeKind::BOOL:
      return true;
    case TypeKind::REF:
      return !type->is_mutable;
    case TypeKind::ARRAY:
    case TypeKind::BOX:
    case TypeKind::VEC:
      return isClone(type->target);
    case TypeKind::STRUCT: {
      const StructInfo* info = structInfo(type->declaration);
      return info != nullptr && info->has_clone;
    }
    default:
      return false;
  }
}

bool Analyzer::isPartialEq(const Type* type) {
  if (type == nullptr) {
    return false;
  }
  switch (type->kind) {
    case TypeKind::I32:
    case TypeKind::U32:
    case TypeKind::ISIZE:
    case TypeKind::USIZE:
    case TypeKind::BOOL:
      return true;
    case TypeKind::REF:
      return isPartialEq(type->target);
    case TypeKind::ARRAY:
    case TypeKind::BOX:
    case TypeKind::VEC:
      return isPartialEq(type->target);
    case TypeKind::STRUCT: {
      const StructInfo* info = structInfo(type->declaration);
      return info != nullptr && info->has_partial_eq;
    }
    default:
      return false;
  }
}

bool Analyzer::isEq(const Type* type) {
  if (type == nullptr) {
    return false;
  }
  switch (type->kind) {
    case TypeKind::I32:
    case TypeKind::U32:
    case TypeKind::ISIZE:
    case TypeKind::USIZE:
    case TypeKind::BOOL:
      return true;
    case TypeKind::REF:
      return isEq(type->target);
    case TypeKind::ARRAY:
    case TypeKind::BOX:
    case TypeKind::VEC:
      return isEq(type->target);
    case TypeKind::STRUCT: {
      const StructInfo* info = structInfo(type->declaration);
      return info != nullptr && info->has_eq;
    }
    default:
      return false;
  }
}

bool Analyzer::isAggregate(const Type* type) {
  return type != nullptr && (type->kind == TypeKind::ARRAY || type->kind == TypeKind::STRUCT);
}

const ast::ConstItem* Analyzer::resolveConstantPath(const ast::Path& path) {
  if (path.size() == 1) {
    auto found = constants_.find(path.front().name);
    return found == constants_.end() ? nullptr : found->second;
  }
  if (path.size() == 2) {
    const StructInfo* info = lookupStruct(path.front());
    if (info == nullptr) {
      return nullptr;
    }
    auto found = info->constants.find(path.back().name);
    return found == info->constants.end() ? nullptr : found->second;
  }
  return nullptr;
}

std::optional<ConstValue> Analyzer::evalConst(const ast::Expr& expression, const Type* expected) {
  return std::visit([&](const auto& value) -> std::optional<ConstValue> {
    using Node = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<Node, ast::IntegerExpr>) {
      const Type* type = nullptr;
      if (!value.suffix.empty()) {
        type = integerType(value.suffix);
      } else if (expected != nullptr && isInteger(expected->kind)) {
        type = expected;
      } else {
        type = make(TypeKind::I32);
      }
      if (type == nullptr) {
        error(expression.span, "invalid integer suffix");
        return std::nullopt;
      }
      return ConstValue{type, parseMagnitude(value.digits, value.base)};
    } else if constexpr (std::is_same_v<Node, ast::BoolExpr>) {
      return ConstValue{make(TypeKind::BOOL), value.is_true ? 1 : 0};
    } else if constexpr (std::is_same_v<Node, ast::PathExpr>) {
      const ast::ConstItem* item = resolveConstantPath(value.path);
      if (item == nullptr) {
        error(expression.span, "a constant context requires a constant path");
        return std::nullopt;
      }
      const ConstValue* resolved = constantValue(*item);
      if (resolved == nullptr) {
        return std::nullopt;
      }
      return *resolved;
    } else if constexpr (std::is_same_v<Node, ast::UnaryExpr>) {
      if (value.op != ast::Op::NEGATE || !value.operand) {
        error(expression.span, "unsupported constant expression");
        return std::nullopt;
      }
      auto magnitude = evalConst(*value.operand, expected);
      if (!magnitude.has_value()) {
        return std::nullopt;
      }
      if (!isSignedKind(magnitude->type->kind)) {
        error(expression.span, "cannot negate an unsigned constant");
        return std::nullopt;
      }
      return ConstValue{magnitude->type, -magnitude->value};
    } else {
      error(expression.span, "unsupported constant expression");
      return std::nullopt;
    }
  }, expression.value);
}

const ConstValue* Analyzer::constantValue(const ast::ConstItem& item) {
  auto cached = const_cache_.find(&item);
  if (cached != const_cache_.end()) {
    return &cached->second;
  }
  if (std::find(const_stack_.begin(), const_stack_.end(), &item) != const_stack_.end()) {
    error(item.span, "constant dependency cycle");
    return nullptr;
  }
  const_stack_.push_back(&item);
  const Type* declared = resolveType(*item.type);
  auto value = declared == nullptr ? std::nullopt : evalConst(*item.value, declared);
  const_stack_.pop_back();
  if (!value.has_value()) {
    return nullptr;
  }
  if (adjustTo(value->type, declared) == nullptr) {
    error(item.value->span, "the initializer does not match the declared type");
    return nullptr;
  }
  auto inserted = const_cache_.emplace(&item, *value);
  return &inserted.first->second;
}

void Analyzer::pushScope() {
  scopes_.emplace_back();
}

void Analyzer::popScope() {
  scopes_.pop_back();
}

LocalVar* Analyzer::lookupLocal(const std::string& name) {
  for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
    auto found = scope->find(name);
    if (found != scope->end()) {
      return &found->second;
    }
  }
  return nullptr;
}
}
