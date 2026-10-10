#pragma once

#include "diagnostic.h"
#include "../ast/ast.h"
#include <istream>
#include <vector>

namespace rx {
enum class Stage { LEX, PARSE, AST };


enum class Entry { CRATE, EXPRESSION, TYPE, ITEM, LET_STATEMENT };

struct FrontendResult
{
  std::vector<Diagnostic> diagnostics;
  std::optional<ast::Root> tree;
};

// No parse-tree pointers escape: their storage belongs to the local parser.
FrontendResult check(
  std::istream& source, const std::string& file, Stage stage, Entry entry = Entry::CRATE);
}
