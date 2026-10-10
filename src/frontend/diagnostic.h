#pragma once

#include <cstddef>
#include <string>

namespace rx {
struct Diagnostic
{
  std::string file;
  std::size_t line;
  std::size_t column;
  std::string stage;
  std::string message;
};
}
