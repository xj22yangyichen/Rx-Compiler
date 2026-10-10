#pragma once

#include "ast.h"
#include <ostream>

namespace rx::ast {
void printAst(std::ostream& output, const Root& root, const std::string& file);
}
