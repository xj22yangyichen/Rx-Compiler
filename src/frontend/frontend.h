#pragma once

#include "../ast/ast.h"
#include "diagnostic.h"
#include "../semantic/result.h"
#include <vector>
#include <iostream>

namespace rx {
enum class Stage { LEX, PARSE, AST, SEMANTIC };


enum class Entry { CRATE, EXPRESSION, TYPE, ITEM, LET_STATEMENT };

struct FrontendResult {
  std::vector<Diagnostic> diagnostics;
  std::optional<ast::Root> tree;
  std::unique_ptr<const semantic::AnalysisResult> semantic;
};

FrontendResult check(std::istream& source, const std::string& file, Stage stage,
  Entry entry = Entry::CRATE);
}
