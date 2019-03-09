#pragma once

#include "ast.h"
#include "lexer.h"

#include <memory>

namespace charlie {

class Parser {
public:
  Parser(std::string file);
  ~Parser();

  std::unique_ptr<Module> parse();

  /*
   * FunctionDefinition ::=
   *      "fn" IDENTIFIER '(' FunctionParameters* ')' IDENTIFIER Block
   */
  std::unique_ptr<FunctionDef> parse_function_definition();
  /*
   * Block ::= '{' Statement* '}'
   */
  std::unique_ptr<Block> parse_block();
  /*
   * Statement ::= BasicStatement ';'
   */
  std::unique_ptr<Statement> parse_statement();
  /*
   * BasicStatement ::=
   *      ReturnStatement
   */
  std::unique_ptr<Statement> parse_basic_statement();
  /*
   * ReturnStatement ::= "return" Expression
   */
  std::unique_ptr<ReturnStatement> parse_return_statement();
  /*
   * Expression ::= IntegerLiteral
   */
  std::unique_ptr<Expression> parse_expression();

  void print_current_token();

private:
  std::string mFileName;
  std::unique_ptr<Lexer> mLexer;
};  // class Parser

}  // namespace charlie
