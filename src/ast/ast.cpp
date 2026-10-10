#include "ast.h"
#include <stdexcept>

namespace rx::ast {
const char* opText(Op op) {
  switch (op) {
    case Op::NEGATE:
    case Op::SUBTRACT:
      return "-";
    case Op::NOT:
      return "!";
    case Op::DEREFERENCE:
    case Op::MULTIPLY:
      return "*";
    case Op::BORROW:
    case Op::BIT_AND:
      return "&";
    case Op::BORROW_MUT:
      return "&mut";
    case Op::ADD:
      return "+";
    case Op::DIVIDE:
      return "/";
    case Op::REMAINDER:
      return "%";
    case Op::BIT_OR:
      return "|";
    case Op::BIT_XOR:
      return "^";
    case Op::SHIFT_LEFT:
      return "<<";
    case Op::SHIFT_RIGHT:
      return ">>";
    case Op::EQUAL:
      return "==";
    case Op::NOT_EQUAL:
      return "!=";
    case Op::LESS:
      return "<";
    case Op::LESS_EQUAL:
      return "<=";
    case Op::GREATER:
      return ">";
    case Op::GREATER_EQUAL:
      return ">=";
    case Op::AND:
      return "&&";
    case Op::OR:
      return "||";
    case Op::ASSIGN:
      return "=";
    case Op::ADD_ASSIGN:
      return "+=";
    case Op::SUBTRACT_ASSIGN:
      return "-=";
    case Op::MULTIPLY_ASSIGN:
      return "*=";
    case Op::DIVIDE_ASSIGN:
      return "/=";
    case Op::REMAINDER_ASSIGN:
      return "%=";
    case Op::BIT_AND_ASSIGN:
      return "&=";
    case Op::BIT_OR_ASSIGN:
      return "|=";
    case Op::BIT_XOR_ASSIGN:
      return "^=";
    case Op::SHIFT_LEFT_ASSIGN:
      return "<<=";
    case Op::SHIFT_RIGHT_ASSIGN:
      return ">>=";
  }
  throw std::logic_error("unknown AST operator");
}
}
