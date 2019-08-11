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
   * TopLevelDeclaration ::= FunctionDefinition
   *
   */
  std::unique_ptr<TopLevelDeclaration> ParseTopLevelDeclaration();

  /*
   * FunctionPrototype ::=
   *      "fn" IDENTIFIER '(' FunctionParameters* ')' IDENTIFIER
   */
  std::unique_ptr<FunctionPrototype> ParseFunctionPrototype();

  /*
   * FunctionDefinition ::= FunctionPrototype Block
   */
  std::unique_ptr<FunctionDefinition> ParseFunctionDefintion();

  /*
   * Block ::= '{' Statement* '}'
   */
  std::unique_ptr<Block> ParseBlock();

  /*
   * Statement ::= BasicStatement ';'
   */
  std::unique_ptr<Statement> ParseStatement();

  /*
   * BasicStatement ::= ReturnStatement
   */
  std::unique_ptr<Statement> ParseBasicStatement();

  /*
   * ReturnStatement ::= "return" Expression
   */
  std::unique_ptr<ReturnStatement> ParseReturnStatement();

  /*
   * Expression ::= IntegerLiteral
   *              | FloatLiteral
   *              | StringLiteral
   */
  std::unique_ptr<Expression> ParseExpression();

private:
  std::string mFileName;
  Lexer mLexer;
};  // class Parser

}  // namespace charlie
