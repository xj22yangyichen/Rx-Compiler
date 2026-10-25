#include "ast/ast-printer.h"
#include "frontend/frontend.h"
#include <string>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
  rx::Stage stage = rx::Stage::PARSE;
  rx::Entry entry = rx::Entry::CRATE;
  std::string file;
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--stage") {
      const std::string value = argv[++i];
      if (value == "lex") {
        stage = rx::Stage::LEX;
      } else if (value == "ast") {
        stage = rx::Stage::AST;
      } else if (value == "semantic") {
        stage = rx::Stage::SEMANTIC;
      }
    } else if (argument == "--entry") {
      const std::string value = argv[++i];
      if (value == "expression") {
        entry = rx::Entry::EXPRESSION;
      } else if (value == "typeRef") {
        entry = rx::Entry::TYPE;
      } else if (value == "item") {
        entry = rx::Entry::ITEM;
      } else if (value == "letStatement") {
        entry = rx::Entry::LET_STATEMENT;
      }
    } else {
      file = argument;
    }
  }
  std::ifstream source(file, std::ios::binary);
  const auto result = rx::check(source, file, stage, entry);
  for (const auto& error : result.diagnostics) {
    std::cerr << error.file << ':' << error.line << ':' << error.column << ": " << error.stage
              << " error: " << error.message << '\n';
  }
  if (result.tree) {
    rx::ast::printAst(std::cout, *result.tree, file);
  }
  return result.diagnostics.empty() ? 0 : 1;
}
