#pragma once

#include <string>
#include <cstddef>

namespace rx {
struct Diagnostic {
  std::string file;
  std::size_t line;
  std::size_t column;
  std::string stage;
  std::string message;
};
}
