#pragma once

#include "ast.h"
#include "lexer.h"

#include <memory>

namespace charlie {

class Parser {
public:
  Parser(std::string file);
  ~Parser();

  std::unique_ptr<Module> Parse();

  /*
   * FunctionDefinition ::=
   *      "fn" IDENTIFIER '(' FunctionParameters* ')' IDENTIFIER Block
   */
  std::unique_ptr<FunctionDef> ParseFunctionDefintion();
  /*
   * Block ::= '{' Statement* '}'
   */
  std::unique_ptr<Block> ParseBlock();
  /*
   * Statement ::= BasicStatement ';'
   */
  std::unique_ptr<Statement> ParseStatement();
  /*
   * BasicStatement ::=
   *      ReturnStatement
   */
  std::unique_ptr<Statement> ParseBasicStatement();
  /*
   * ReturnStatement ::= "return" Expression
   */
  std::unique_ptr<ReturnStatement> ParseReturnStatement();
  /*
   * Expression ::= IntegerLiteral
   */
  std::unique_ptr<Expression> ParseExpression();

private:
  std::string mFileName;
  Lexer mLexer;
};  // class Parser

}  // namespace charlie
