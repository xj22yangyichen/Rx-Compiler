#include "frontend/frontend.h"
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
constexpr const char* USAGE =
  "Usage: rxc [--stage lex|parse] [--entry crate|expression|typeRef|item|letStatement] <source.rx>\n";
}

int main(int argc, char** argv) {
  try {
    rx::Stage stage = rx::Stage::PARSE;
    rx::Entry entry = rx::Entry::CRATE;
    bool has_entry = false;
    std::string file;
    for (int i = 1; i < argc; ++i) {
      const std::string argument = argv[i];
      if (argument == "--stage" && i + 1 < argc) {
        const std::string value = argv[++i];
        if (value == "lex") stage = rx::Stage::LEX;
        else if (value == "parse") stage = rx::Stage::PARSE;
        else {
          std::cerr << "unsupported stage: " << value << '\n';
          return 2;
        }
      } else if (argument == "--entry" && i + 1 < argc) {
        const std::string value = argv[++i];
        has_entry = true;
        if (value == "crate") entry = rx::Entry::CRATE;
        else if (value == "expression") entry = rx::Entry::EXPRESSION;
        else if (value == "typeRef") entry = rx::Entry::TYPE;
        else if (value == "item") entry = rx::Entry::ITEM;
        else if (value == "letStatement") entry = rx::Entry::LET_STATEMENT;
        else {
          std::cerr << "unsupported entry: " << value << '\n';
          return 2;
        }
      } else if (argument == "--help") {
        std::cout << USAGE;
        return 0;
      } else if (!argument.empty() && argument[0] != '-' && file.empty()) {
        file = argument;
      } else {
        std::cerr << "invalid argument: " << argument << '\n';
        return 2;
      }
    }
    if (file.empty()) {
      std::cerr << USAGE;
      return 2;
    }
    if (has_entry && stage == rx::Stage::LEX) {
      std::cerr << "--entry is only valid with the parse stage\n";
      return 2;
    }

    std::ifstream source(file, std::ios::binary);
    if (!source || !std::filesystem::is_regular_file(file)) {
      std::cerr << "cannot read source: " << file << '\n';
      return 2;
    }
    const auto diagnostics = rx::check(source, file, stage, entry);
    if (source.bad()) {
      std::cerr << "error reading source: " << file << '\n';
      return 2;
    }
    for (const auto& error : diagnostics) {
      std::cerr << error.file << ':' << error.line << ':' << error.column
        << ": " << error.stage << " error: " << error.message << '\n';
    }
    return diagnostics.empty() ? 0 : 1;
  } catch (const std::exception& error) {
    std::cerr << "frontend failure: " << error.what() << '\n';
    return 2;
  }
}
