#pragma once

#include "diagnostic.h"
#include <istream>
#include <vector>

namespace rx {
enum class Stage { LEX, PARSE };


enum class Entry { CRATE, EXPRESSION, TYPE, ITEM, LET_STATEMENT };

// No parse-tree pointers escape: their storage belongs to the local parser.
std::vector<Diagnostic> check(std::istream& source, const std::string& file,
  Stage stage, Entry entry = Entry::CRATE);
}
