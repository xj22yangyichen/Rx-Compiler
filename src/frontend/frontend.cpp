#include "RxLexer.h"
#include "frontend.h"
#include "RxParser.h"
#include "ast-builder.h"
#include "error-listener.h"
#include "../semantic/analyzer.h"

namespace rx {
namespace {
const char* invalidToken(std::size_t type) {
  switch (type) {
    case RxLexer::INVALID_LIFETIME:
      return "invalid lifetime token";
    case RxLexer::INVALID_CHARACTER_LITERAL:
      return "unsupported character literal";
    case RxLexer::INVALID_NUMBER:
      return "invalid integer literal";
    case RxLexer::UNTERMINATED_BLOCK_COMMENT:
      return "unterminated block comment";
    case RxLexer::ERROR_CHAR:
      return "unrecognized character";
    default:
      return nullptr;
  }
}
}

FrontendResult check(std::istream& source, const std::string& file, Stage stage, Entry entry) {
  FrontendResult result;
  auto& diagnostics = result.diagnostics;
  ErrorListener lexer_errors(file, "lex", diagnostics);
  ErrorListener parser_errors(file, "parse", diagnostics);
  antlr4::ANTLRInputStream input(source);
  RxLexer lexer(&input);
  lexer.removeErrorListeners();
  lexer.addErrorListener(&lexer_errors);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  for (auto* token : tokens.getTokens()) {
    if (const auto* message = invalidToken(token->getType())) {
      diagnostics.push_back({file, token->getLine(), token->getCharPositionInLine() + 1,
        "lex", message});
    }
  }
  if (!diagnostics.empty() || stage == Stage::LEX) {
    return result;
  }

  tokens.seek(0);
  RxParser parser(&tokens);
  parser.removeErrorListeners();
  parser.addErrorListener(&parser_errors);
  antlr4::ParserRuleContext* tree = nullptr;
  switch (entry) {
    case Entry::CRATE:
      tree = parser.crate();
      break;
    case Entry::EXPRESSION:
      tree = parser.expression();
      break;
    case Entry::TYPE:
      tree = parser.typeRef();
      break;
    case Entry::ITEM:
      tree = parser.item();
      break;
    case Entry::LET_STATEMENT:
      tree = parser.letStatement();
      break;
  }

  if (diagnostics.empty() && tokens.LA(1) != antlr4::Token::EOF) {
    auto* token = tokens.LT(1);
    diagnostics.push_back({file, token->getLine(), token->getCharPositionInLine() + 1,
      "parse", "unexpected trailing input"});
  }
  // Error recovery of ANTLR can generate a CST even if the code is invalid,
  // so acceptance depends on diagnostics, not the CST.
  if (diagnostics.empty() && (stage == Stage::AST || stage == Stage::SEMANTIC)) {
    result.tree = AstBuilder().build_root(tree);
    if (stage == Stage::SEMANTIC) {
      semantic::analyze(*std::get<std::unique_ptr<ast::Crate>>(*result.tree), file, diagnostics);
    }
  }
  return result;
}
}
