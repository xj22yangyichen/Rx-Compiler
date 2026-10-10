#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace rx::ast {
struct Span
{
  std::size_t begin = 0;
  std::size_t end = 0;
  std::size_t line = 1;
  std::size_t column = 1;
};


struct Expr;
struct Type;
struct Stmt;
struct Item;
using ExprPtr = std::unique_ptr<Expr>;
using TypePtr = std::unique_ptr<Type>;
using StmtPtr = std::unique_ptr<Stmt>;
using ItemPtr = std::unique_ptr<Item>;


enum class Op {
  NEGATE, NOT, DEREFERENCE, BORROW, BORROW_MUT, ADD, SUBTRACT, MULTIPLY,
  DIVIDE, REMAINDER, BIT_AND, BIT_OR, BIT_XOR, SHIFT_LEFT, SHIFT_RIGHT, EQUAL,
  NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, AND, OR, ASSIGN,
  ADD_ASSIGN, SUBTRACT_ASSIGN, MULTIPLY_ASSIGN, DIVIDE_ASSIGN,
  REMAINDER_ASSIGN, BIT_AND_ASSIGN, BIT_OR_ASSIGN, BIT_XOR_ASSIGN,
  SHIFT_LEFT_ASSIGN, SHIFT_RIGHT_ASSIGN
};


struct PathSegment
{
  Span span;
  std::string name;
  std::vector<TypePtr> arguments;
};
using Path = std::vector<PathSegment>;


struct PathType
{
  Path path;
};


struct RefType
{
  bool is_mutable;
  TypePtr inner;
};


struct ArrayType
{
  TypePtr element;
  ExprPtr length;
};


struct UnitType
{
};


struct Type
{
  Span span;
  std::variant<PathType, RefType, ArrayType, UnitType> value;
};


struct IntegerExpr
{
  std::string spelling;
  std::string digits;
  unsigned base;
  std::string suffix;
};


struct BoolExpr
{
  bool is_true;
};


struct UnitExpr
{
};


struct PathExpr
{
  Path path;
};


struct UnaryExpr
{
  Op op;
  ExprPtr operand;
};


struct BinaryExpr
{
  Op op;
  ExprPtr left;
  ExprPtr right;
};


struct AssignExpr
{
  Op op;
  ExprPtr left;
  ExprPtr right;
};


struct CastExpr
{
  ExprPtr expression;
  TypePtr type;
};


struct CallExpr
{
  ExprPtr callee;
  std::vector<ExprPtr> arguments;
};


struct MethodCallExpr
{
  ExprPtr receiver;
  PathSegment method;
  std::vector<ExprPtr> arguments;
};


struct FieldExpr
{
  ExprPtr object;
  std::string name;
};


struct IndexExpr
{
  ExprPtr object;
  ExprPtr index;
};


struct FieldInit
{
  Span span;
  std::string name;
  ExprPtr value;
};


struct StructExpr
{
  Path path;
  std::vector<FieldInit> fields;
};


struct ArrayExpr
{
  std::vector<ExprPtr> elements;
};


struct RepeatExpr
{
  ExprPtr element;
  ExprPtr length;
};


struct BlockExpr
{
  std::vector<StmtPtr> statements;
  ExprPtr tail;
};


struct IfExpr
{
  ExprPtr condition;
  ExprPtr then_branch;
  ExprPtr else_branch;
};


struct LoopExpr
{
  ExprPtr body;
};


struct WhileExpr
{
  ExprPtr condition;
  ExprPtr body;
};


struct BreakExpr
{
  ExprPtr value;
};


struct ContinueExpr
{
};


struct ReturnExpr
{
  ExprPtr value;
};


struct Expr
{
  Span span;
  std::variant<IntegerExpr, BoolExpr, UnitExpr, PathExpr, UnaryExpr,
    BinaryExpr, AssignExpr, CastExpr, CallExpr, MethodCallExpr, FieldExpr,
    IndexExpr, StructExpr, ArrayExpr, RepeatExpr, BlockExpr, IfExpr, LoopExpr,
    WhileExpr, BreakExpr, ContinueExpr, ReturnExpr> value;
};


struct LetStmt
{
  std::string name;
  bool is_mutable;
  TypePtr type;
  ExprPtr init;
};


struct ExprStmt
{
  ExprPtr expression;
  bool has_semicolon;
};


struct EmptyStmt
{
};


struct Stmt
{
  Span span;
  std::variant<LetStmt, ExprStmt, EmptyStmt> value;
};


struct Receiver
{
  Span span;
  bool is_reference;
  bool is_mutable;
};


struct Parameter
{
  Span span;
  std::string name;
  bool is_mutable;
  TypePtr type;
};


struct FunctionItem
{
  std::string name;
  std::optional<Receiver> receiver;
  std::vector<Parameter> parameters;
  TypePtr return_type;
  ExprPtr body;
};


struct StructField
{
  Span span;
  std::string name;
  TypePtr type;
};


struct StructItem
{
  std::string name;
  std::vector<std::string> derives;
  std::vector<StructField> fields;
};


struct ConstItem
{
  std::string name;
  TypePtr type;
  ExprPtr value;
};


struct ImplItem
{
  TypePtr type;
  std::vector<ItemPtr> items;
};


struct UseItem
{
};


struct Item
{
  Span span;
  std::variant<FunctionItem, StructItem, ConstItem, ImplItem, UseItem> value;
};


struct Crate
{
  Span span;
  std::vector<ItemPtr> items;
};
using Root = std::variant<std::unique_ptr<Crate>, ItemPtr, ExprPtr, TypePtr, StmtPtr>;

template <class Node, class Value>
std::unique_ptr<Node> makeNode(Span span, Value value) {
  return std::make_unique<Node>(Node{span, std::move(value)});
}

const char* opText(Op op);
}
