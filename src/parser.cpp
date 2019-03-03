#include <cassert>
#include <iostream>
#include <variant>
#include "parser.h"

namespace charlie {

Parser::Parser(std::string file)
    : mFileName(std::move(file)), mLexer(std::make_unique<Lexer>(mFileName)) {}

Parser::~Parser() {}

std::unique_ptr<Module> Parser::parse() {
  auto func_def = parse_function_definition();
  if (!func_def)
    return nullptr;
  return std::make_unique<Module>(std::string(mFileName), std::move(func_def));
}

/*
 * FunctionDeclaration ::= "fn" IDENTIFIER '(' FunctionParameters* ')' IDENTIFIER Block
 */
std::unique_ptr<FunctionDef> Parser::parse_function_definition() {
  // "fun"
  mLexer->expect_token(TOK_KEYWORD_FUN);
  print_current_token();

  // IDENTIFIER (function name)
  mLexer->next_token();
  std::string fn_name;
  if (mLexer->is_identifier()) {
    fn_name = std::get<Lexer::vIdentifier>(mLexer->mCurrentValue);
    std::cout << "DEBUG: Lexer consumed identifier: " << fn_name << '\n';
  }
  print_current_token();

  // '('
  mLexer->expect_token(TOK_PAREN_LEFT);
  print_current_token();

  std::vector<std::string> args;
  // parse_function_parameters();

  // ')'
  mLexer->expect_token(TOK_PAREN_RIGHT);
  print_current_token();

  // IDENTIFIER (return type)
  mLexer->next_token();
  std::string return_type;
  if (mLexer->is_identifier()) {
    return_type = std::get<Lexer::vIdentifier>(mLexer->mCurrentValue);
    std::cout << "DEBUG: Lexer consumed identifier: " << return_type << '\n';
  }

  // Block
  auto block = parse_block();

  if (!fn_name.empty() && !return_type.empty() && block) {
    return std::make_unique<FunctionDef>(std::move(fn_name),
                                         std::move(return_type),
                                         std::move(args), std::move(block));
  } else {
    return nullptr;
  }
}

/*
 * Block ::= '{' Statement* '}'
 */
std::unique_ptr<Block> Parser::parse_block() {
  // '{'
  mLexer->expect_token(TOK_BRACE_LEFT);
  print_current_token();

  // TODO: Blocks need to parse multiple statements
  std::vector<std::unique_ptr<Statement>> stmts;
  auto stmt = parse_statement();
  if (stmt)
    stmts.push_back(std::move(stmt));

  // '}'
  mLexer->expect_token(TOK_BRACE_RIGHT);
  print_current_token();

  return std::make_unique<Block>(std::move(stmts));
}

/*
 * Statement ::= BasicStatement ';'
 */
std::unique_ptr<Statement> Parser::parse_statement() {
  // BasicStatement
  std::unique_ptr<Statement> stmt = parse_basic_statement();
  if (!stmt)
    return nullptr;

  // ';'
  mLexer->expect_token(TOK_SEMICOLON);
  print_current_token();
  return std::move(stmt);
}

 /*
  * BasicStatement ::=
  *      ReturnStatement
  */
std::unique_ptr<Statement> Parser::parse_basic_statement() {
  std::unique_ptr<Statement> return_stmt = parse_return_statement();
  if (!return_stmt)
    return nullptr;
  return std::move(return_stmt);
}

/*
 * ReturnStatement ::= "return" Expression
 */
std::unique_ptr<ReturnStatement> Parser::parse_return_statement() {
  mLexer->expect_token(TOK_KEYWORD_RETURN);
  print_current_token();
  std::unique_ptr<Expression> expr = parse_expression();
  if (!expr)
    return nullptr;
  return std::make_unique<ReturnStatement>(std::move(expr));
}

/*
 * Expression ::=
 *      | IntegerLiteral
 *      | FloatLiteral
 */
std::unique_ptr<Expression> Parser::parse_expression() {
  mLexer->next_token();
  if (mLexer->is_int()) {
    int i = std::get<Lexer::vIntLiteral>(mLexer->mCurrentValue);
    std::cout << "DEBUG: Lexer consumed int: " << i << '\n';
    return std::make_unique<IntegerLiteral>(i);
  } else if (mLexer->is_float()) {
    float f = std::get<Lexer::vFloatLiteral>(mLexer->mCurrentValue);
    std::cout << "DEBUG: Lexer consumed float: " << f << '\n';
    print_current_token();
    return std::make_unique<FloatLiteral>(f);
  } else {
    std::cout << "DEBUG: Lexer did not consume experssion\n";
    return nullptr;
  }
}

void Parser::print_current_token() {
    std::cout << "Token line number: "   << mLexer->mCurrentToken.line       << '\n'
              << "Token starts at pos: " << mLexer->mCurrentToken.start_pos  << '\n'
              << "Token has kind: "      << mLexer->mCurrentToken.kind       << '\n'
              << "====="                                                     << '\n';
}

} // namesapce charlie
