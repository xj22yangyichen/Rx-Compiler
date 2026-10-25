#pragma once

#include "type.h"
#include "../ast/ast.h"
#include "../frontend/diagnostic.h"
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>

namespace rx::semantic {
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
};


struct ConstValue {
  const Type* type = nullptr;
  std::int64_t value = 0;
};


struct Place {
  const Type* type = nullptr;
  bool is_mutable = false;
  bool is_borrowed_shared = false;
};


struct LoopContext {
  bool accepts_value = false;
  const Type* expected = nullptr;
  std::vector<const Type*> break_types;
  bool has_break = false;
};


class Analyzer {
public:
  // 整个语义检查部分的入口
  void run(const ast::Crate& crate, const std::string& file, std::vector<Diagnostic>& diagnostics);

private:
  // 保存文件名用于报错
  std::string file_;
  // 保存语义检查阶段的报错信息列表
  std::vector<Diagnostic>* diagnostics_ = nullptr;
  // 分析过程中产生的类型对象都放在 arena_ 中，避免频繁的内存分配
  std::deque<Type> arena_;
  // 结构体名字到结构体信息的映射表
  std::map<std::string, StructInfo> structs_;
  // 函数名字到函数声明的映射表
  std::map<std::string, const ast::FunctionItem*> functions_;
  // 常量名字到常量声明的映射表
  std::map<std::string, const ast::ConstItem*> constants_;
  // 常量声明到常量值的映射表，用于缓存常量的求值结果，以防止重复求值
  std::map<const ast::ConstItem*, ConstValue> const_cache_;
  // 常量求值时的依赖栈，如果已有的常量再次出现，说明存在循环依赖
  std::vector<const ast::ConstItem*> const_stack_;
  // 局部变量的作用域栈，每个作用域是一个名字到局部变量的映射表
  std::vector<std::map<std::string, LocalVar>> scopes_;
  // 循环上下文栈，用于处理 break/continue 的类型检查
  std::vector<LoopContext> loops_;
  // 保存进入条件语句时的栈深度，用于检查 break/continue 是否在条件语句中使用
  std::vector<std::size_t> condition_barriers_;
  // 指向当前 Self 所代表的结构体信息
  const StructInfo* self_struct_ = nullptr;
  // 指向当前函数的返回类型，用于检查 return 语句的类型是否正确
  const Type* return_type_ = nullptr;

  // 把语义检查阶段的报错记录到之前准备好的 diagnostics_ 中
  void error(const ast::Span& span, const std::string& message);

  // 创建指定种类的语义类型，加入 arena_ 中，返回指向它的指针
  const Type* make(TypeKind kind);
  // 识别整数类型，调用 make 创建对应的语义类型
  const Type* integerType(const std::string& name);
  // 调用 make 创建数组类型
  const Type* arrayType(const Type* element, std::size_t length);
  // 调用 make 创建引用类型
  const Type* referenceType(const Type* target, bool is_mutable);
  // 调用 make 创建容器类型，主要用于 Vec<T>、Box<T>
  const Type* containerType(TypeKind kind, const Type* element);
  // 调用 make 创建结构体类型
  const Type* structType(const StructInfo& info);
  // 将 AST 中的类型解析为语义类型
  const Type* resolveType(const ast::Type& type);
  // 将 AST 中的路径解析为语义类型
  const Type* resolvePathType(const ast::Path& path);
  // 根据 AST 中的路径段查找对应的结构体信息
  const StructInfo* lookupStruct(const ast::PathSegment& segment);
  // 从结构体声明中获取对应的语义信息
  const StructInfo* structInfo(const ast::StructItem* declaration);
  // 查找一个 impl 声明所属的结构体信息
  const StructInfo* implTarget(const ast::ImplItem& impl);

  // 收集顶层结构体声明
  void collectStructs(const ast::Crate& crate);
  // 收集顶层函数和常量声明
  void collectValues(const ast::Crate& crate);
  // 收集结构体的关联项
  void collectImpls(const ast::Crate& crate);
  // 登记一个结构体
  void declareStruct(const ast::StructItem& item);
  // 登记一个顶层函数
  void declareFunction(const ast::FunctionItem& item);
  // 登记一个顶层常量
  void declareConstant(const ast::ConstItem& item);
  // 共两轮，一轮检查能力依赖，一轮检查结构体自身的每个字段是否都支持该结构体声明的能力
  void checkDerives();
  // 检查结构体的内存布局是否存在循环引用
  void checkLayout();
  // 递归检测结构体的字段类型是否存在循环引用
  bool hasInlineCycle(const StructInfo& info, std::vector<const StructInfo*>& stack);
  // 在顶层函数中检查 main 函数是否存在、是否没有参数、是否返回 ()
  void checkEntry(const ast::Crate& crate);
  // 检查函数签名和常量定义
  void checkSignatures(const ast::Crate& crate);
  // 检查单个函数的签名
  void checkSignature(const ast::FunctionItem& item);
  // 检查单个函数的函数体
  void checkFunction(const ast::FunctionItem& item);

  // 判断类型是否支持 Copy
  bool isCopy(const Type* type);
  // 判断类型是否支持 Clone
  bool isClone(const Type* type);
  // 判断类型是否支持 PartialEq
  bool isPartialEq(const Type* type);
  // 判断类型是否支持 Eq
  bool isEq(const Type* type);
  // 判断类型是否为数组或结构体
  bool isAggregate(const Type* type);

  // 对常量表达式进行求值
  std::optional<ConstValue> evalConst(const ast::Expr& expression, const Type* expected);
  // 获取一个常量的值，如果该常量已经求值过，则直接返回缓存的结果，
  // 否则用 const_stack_ 检测循环依赖，解析声明类型，
  // 以该类型为期望调用 evalConst 求值，并将结果缓存到 const_cache_ 中
  const ConstValue* constantValue(const ast::ConstItem& item);
  // 把常量路径解析为对应的常量声明
  const ast::ConstItem* resolveConstantPath(const ast::Path& path);

  // 进入一个作用域
  void pushScope();
  // 离开当前作用域
  void popScope();
  // 查找局部变量，返回第一个匹配的局部变量（内层遮蔽外层）
  LocalVar* lookupLocal(const std::string& name);

  // 检查块表达式：检查块中的每个语句，再检查尾表达式，如有返回其类型，如无则根据是否发散返回 () 或 !
  const Type* checkBlock(const ast::BlockExpr& block, const Type* expected, bool* diverges = nullptr);
  // 检查语句：空语句返回 ()，let 语句交给 checkLet，表达式语句交给 checkValue
  const Type* checkStmt(const ast::Stmt& statement, bool* diverges = nullptr);
  // 检查局部变量声明和初始化表达式，并将局部变量加入当前作用域
  const Type* checkLet(const ast::LetStmt& binding, bool* diverges = nullptr);
  // 检查值表达式及其期望类型
  const Type* checkValue(const ast::Expr& expression, const Type* expected, bool* diverges);

  // 表达式检查的入口
  const Type* checkExpr(const ast::Expr& expression, const Type* expected);
  // 确定整数字面量的类型
  const Type* checkIntegerLiteral(const ast::IntegerExpr& literal, const Type* expected);
  //确定名字或路径表达式的类型
  const Type* checkPathExpr(const ast::PathExpr& path);
  // 检查一元表达式
  const Type* checkUnary(const ast::UnaryExpr& unary);
  // 检查二元表达式
  const Type* checkBinary(const ast::BinaryExpr& binary);
  // 检查普通赋值和复合赋值表达式
  const Type* checkAssign(const ast::AssignExpr& assign);
  // 检查显式 as 转换
  const Type* checkCast(const ast::CastExpr& cast);
  // 解析并检查路径形式的函数调用
  const Type* checkCall(const ast::CallExpr& call);
  // 检查点号调用形式的方法调用
  const Type* checkMethodCall(const ast::MethodCallExpr& call);
  // 检查字段访问表达式并返回字段的类型
  const Type* checkField(const ast::FieldExpr& field);
  // 检查索引访问表达式并返回元素的类型
  const Type* checkIndex(const ast::IndexExpr& index);
  // 检查结构体构造表达式
  const Type* checkStructExpr(const ast::StructExpr& literal);
  // 检查列表式数组 [a, b, c]
  const Type* checkArray(const ast::ArrayExpr& array, const Type* expected);
  // 检查重复式数组 [x; n]
  const Type* checkRepeat(const ast::RepeatExpr& repeat, const Type* expected);
  // 检查 if 表达式以及两个分支的结果类型
  const Type* checkIf(const ast::IfExpr& branch, const Type* expected);
  // 检查可带值退出的 loop 表达式
  const Type* checkLoop(const ast::LoopExpr& loop, const Type* expected);
  // 检查 while 循环
  const Type* checkWhile(const ast::WhileExpr& loop);
  // 检查 break 并登记循环退出信息
  const Type* checkBreak(const ast::BreakExpr& jump);
  // 检查 continue 的目标是否合法
  const Type* checkContinue(const ast::ContinueExpr& jump);
  // 检查函数返回表达式
  const Type* checkReturn(const ast::ReturnExpr& jump);
  // 检查普通函数调用，根据已找到的函数声明和接收者类型，检查参数类型是否匹配，并返回函数的返回类型
  const Type* checkOrdinaryCall(const ast::CallExpr& call,
    const ast::FunctionItem& function, const Type* receiver_type);

  // 位置分析的入口
  std::optional<Place> checkPlace(const ast::Expr& expression);
  // 将局部变量的名字解释为位置
  std::optional<Place> placeOfPath(const ast::PathExpr& path);
  // 计算字段访问表达式的位置属性
  std::optional<Place> placeOfField(const ast::FieldExpr& field);
  // 计算数组或 Vec 元素的位置属性
  std::optional<Place> placeOfIndex(const ast::IndexExpr& index);
  // 计算显式解引用 *value 的位置属性
  std::optional<Place> placeOfUnary(const ast::UnaryExpr& unary);

  // 检查隐式类型转换，失败时报错
  bool coerce(const ast::Expr& expression, const Type* from, const Type* to);
  // 判断一种类型能否调整为另一种类型
  const Type* adjustTo(const Type* from, const Type* to);
  // 计算一组类型的最小上界（即共同可以转换到的类型），用于 if/else 分支的类型推断
  const Type* lubOf(const std::vector<const Type*>& types);
  // 解引用一次后的类型
  const Type* derefOnce(const Type* type);

  // 查找与接收者候选类型匹配的固有方法，返回对应的函数信息
  const FunctionInfo* findInherent(const Type* candidate, const std::string& name);

  // 要求一个位置可变，否则报错
  void requireMutable(const ast::Span& span, const Place& place, const std::string& what);
  // 检查表达式是否为布尔类型
  bool expectBool(const ast::Expr& expression, const std::string& context);
  // 根据类型和字段名查找结构体的字段类型，如果不存在则报错
  const Type* fieldOf(const Type* type, const std::string& name, const ast::Span& span);
  // 如果位置已经经过共享借用，而该位置的类型又是 &mut T，就将结果中的类型调整为 &T
  Place narrowPlace(Place place);
};

// 语义分析的入口，创建一个 Analyzer 对象并调用 run 方法
void analyze(const ast::Crate& crate, const std::string& file, std::vector<Diagnostic>& diagnostics);
}
