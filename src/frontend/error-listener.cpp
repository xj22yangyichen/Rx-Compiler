#include "error-listener.h"
#include <utility>

namespace rx {
ErrorListener::ErrorListener(std::string file, std::string stage,
  std::vector<Diagnostic>& diagnostics)
  : file_(std::move(file)), stage_(std::move(stage)), diagnostics_(diagnostics) {}

void ErrorListener::syntaxError(antlr4::Recognizer*, antlr4::Token*,
  std::size_t line, std::size_t column, const std::string& message, std::exception_ptr) {
  diagnostics_.push_back({file_, line, column + 1, stage_, message});
}
}
