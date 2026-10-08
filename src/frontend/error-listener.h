#pragma once

#include "diagnostic.h"
#include <antlr4-runtime.h>
#include <exception>
#include <vector>

namespace rx {
class ErrorListener final : public antlr4::BaseErrorListener {
public:
  ErrorListener(std::string file, std::string stage, std::vector<Diagnostic>& diagnostics);
  // ANTLR requires this callback name.
  void syntaxError(antlr4::Recognizer*, antlr4::Token*, std::size_t line,
    std::size_t column, const std::string& message, std::exception_ptr) override;

private:
  std::string file_;
  std::string stage_;
  std::vector<Diagnostic>& diagnostics_;
};
}
