#pragma once

#include "diagnostic.h"
#include "../ast/ast.h"
#include <vector>
#include <iostream>

namespace rx {
enum class Stage { LEX, PARSE, AST, SEMANTIC };


enum class Entry { CRATE, EXPRESSION, TYPE, ITEM, LET_STATEMENT };

struct FrontendResult {
  std::vector<Diagnostic> diagnostics;
  std::optional<ast::Root> tree;
};

FrontendResult check(std::istream& source, const std::string& file, Stage stage,
  Entry entry = Entry::CRATE);
}
