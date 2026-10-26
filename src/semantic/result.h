#pragma once

#include "type.h"
#include "../ast/ast.h"
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <variant>
#include <cstdint>
#include <optional>

namespace rx::semantic {
// 保存某个值名称对应的声明
using ValueBinding = std::variant<const ast::LetStmt*, const ast::Parameter*,
  const ast::Receiver*, const ast::ConstItem*>;
// 保存循环语句的跳转目标
using LoopTarget = std::variant<const ast::LoopExpr*, const ast::WhileExpr*>;

struct FunctionInfo {
  const ast::FunctionItem* declaration = nullptr;
};


struct StructInfo {
  ast::Span span;
  const ast::StructItem* declaration = nullptr;
  std::vector<const Type*> field_types;
  std::map<std::string, FunctionInfo> functions;
  std::map<std::string, const ast::ConstItem*> constants;
  bool has_copy = false;
  bool has_clone = false;
  bool has_partial_eq = false;
  bool has_eq = false;
};


struct LocalVar {
  const Type* type = nullptr;
  bool is_mutable = false;
  ValueBinding binding = {};
};


struct ConstValue {
  const Type* type = nullptr;
  std::int64_t value = 0;
};


// 描述可寻址的位置
struct Place {
  const Type* type = nullptr;
  bool is_mutable = false;
  bool is_borrowed_shared = false;
};


// 隐式调整种类，分别为解引用、共享借用、可变借用、将 never 适配到目标类型
enum class AdjustmentKind { DEREFERENCE, BORROW, BORROW_MUT, NEVER };


// 一步调整。保存操作和这一步后的类型
struct Adjustment {
  AdjustmentKind kind;
  const Type* target = nullptr;
};


struct ExpressionInfo {
  const Type* type = nullptr;
  std::optional<Place> place;
  std::vector<Adjustment> adjustments;
};


// 标识 Rx 内建操作
enum class Builtin {
  NONE, GET_I32, PRINT_I32, PRINTLN_I32, BOX_NEW, VEC_NEW,
  CLONE, LEN, IS_EMPTY, PUSH, REMOVE
};


struct CallInfo {
  const ast::FunctionItem* function = nullptr;
  Builtin builtin = Builtin::NONE;
  const Type* owner = nullptr;
  const Type* receiver = nullptr;
  std::vector<Adjustment> receiver_adjustments;
};


struct FunctionSignature {
  const Type* receiver = nullptr;
  std::vector<const Type*> parameters;
  const Type* result = nullptr;
};


struct FieldInfo {
  const ast::StructItem* owner = nullptr;
  std::size_t index = 0;
};


// 最终交给 IR 的语义分析结果集合
struct AnalysisResult {
  AnalysisResult() = default;
  AnalysisResult(const AnalysisResult&) = delete;
  AnalysisResult& operator=(const AnalysisResult&) = delete;

  // 保存所有类型的内存池，保证指针有效性
  std::deque<Type> types;
  // 表达式节点到类型、可寻址位置和调整的映射
  std::map<const ast::Expr*, ExpressionInfo> expressions;
  // AST 类型节点到解析后的语义类型的映射
  std::map<const ast::Type*, const Type*> resolved_types;
  // 路径表达式到具体声明的映射
  std::map<const ast::PathExpr*, ValueBinding> bindings;
  // let 声明到局部变量信息的映射
  std::map<const ast::LetStmt*, LocalVar> locals;
  // 函数声明到函数签名的映射
  std::map<const ast::FunctionItem*, FunctionSignature> functions;
  // 结构体名称到结构体信息的映射
  std::map<std::string, StructInfo> structs;
  // 常量声明到求值结果的映射
  std::map<const ast::ConstItem*, ConstValue> constants;
  // 常量表达式到求值结果的映射
  std::map<const ast::Expr*, ConstValue> constant_expressions;
  // 普通调用节点到调用信息的映射
  std::map<const ast::CallExpr*, CallInfo> calls;
  // 方法调用节点到调用信息（目标和接收者调整）的映射
  std::map<const ast::MethodCallExpr*, CallInfo> methods;
  // 字段访问节点到字段信息的映射
  std::map<const ast::FieldExpr*, FieldInfo> fields;
  // 结构体初始化节点到字段信息的映射
  std::map<const ast::FieldInit*, FieldInfo> initializers;
  // 分支语句节点是否发散的映射
  std::map<const ast::BlockExpr*, bool> block_divergence;
  // break 节点要退出的循环
  std::map<const ast::BreakExpr*, LoopTarget> breaks;
  // continue 节点要继续的循环
  std::map<const ast::ContinueExpr*, LoopTarget> continues;
  // return 节点所属的函数
  std::map<const ast::ReturnExpr*, const ast::FunctionItem*> returns;
};
}
