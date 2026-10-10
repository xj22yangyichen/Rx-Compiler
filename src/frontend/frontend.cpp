#include "frontend.h"
#include "error-listener.h"
#include "ast-builder.h"
#include "RxLexer.h"
#include "RxParser.h"

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

  // These rules successfully match invalid input, so no lexer callback fires.
  for (auto* token : tokens.getTokens()) {
    if (const auto* message = invalidToken(token->getType())) {
      diagnostics.push_back(
        {file, token->getLine(), token->getCharPositionInLine() + 1, "lex", message});
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

  // Fragment rules do not consume EOF: reject any unparsed suffix.
  if (diagnostics.empty() && tokens.LA(1) != antlr4::Token::EOF) {
    auto* token = tokens.LT(1);
    diagnostics.push_back({file,
      token->getLine(),
      token->getCharPositionInLine() + 1,
      "parse",
      "unexpected trailing input"});
  }
  // Error recovery can return a tree; acceptance depends on diagnostics, not the tree.
  if (diagnostics.empty() && stage == Stage::AST) {
    result.tree = AstBuilder().build_root(tree);
  }
  return result;
}
}
