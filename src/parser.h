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
   * TopLevelDeclaration ::= ProcedureDefinition
   */
  std::unique_ptr<TopLevelDeclaration> ParseTopLevelDeclaration();

  /*
   * ProcedurePrototype ::=
   *     IDENTIFIER "::" "proc" '(' { ProcdeureParameters } ')' [ "->" IDENTIFIER ]
   */
  std::unique_ptr<ProcedurePrototype> ParseProcedurePrototype(std::string proc_name);

  /*
   * ProcedureDefinition ::= ProcedurePrototype Block
   */
  std::unique_ptr<ProcedureDefinition> ParseProcedureDefintion(std::string proc_name);

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
