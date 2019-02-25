#include <cassert>
#include <iostream>
#include <variant>
#include "parser.h"

namespace charlie {

Parser::Parser(const std::string &file) :
    mLexer(std::make_unique<Lexer>(file))
    {}

Parser::~Parser() {}

void Parser::parse() {
    parse_function_declaration();
}

/*
 * FunctionDeclaration ::= "fn" IDENTIFIER '(' FunctionParameters* ')' IDENTIFIER Block
 */
void Parser::parse_function_declaration() {
    // "fun"
    mLexer->expect_token(TOK_KEYWORD_FUN);
    print_current_token();

    // IDENTIFIER (function name)
    mLexer->next_token();
    if(mLexer->is_identifier()) {
        std::string &s = std::get<Lexer::vIdentifier>(mLexer->mCurrentValue);
        std::cout << "DEBUG: Lexer consumed identifier: " << s << '\n';
    }
    print_current_token();

    // '('
    mLexer->expect_token(TOK_PAREN_LEFT);
    print_current_token();
    // parse_function_parameters();

    // ')'
    mLexer->expect_token(TOK_PAREN_RIGHT);
    print_current_token();

    // IDENTIFIER (return type)
    mLexer->next_token();
    if(mLexer->is_identifier()) {
        std::string &s = std::get<Lexer::vIdentifier>(mLexer->mCurrentValue);
        std::cout << "DEBUG: Lexer consumed identifier: " << s << '\n';
    }

    // Block
    parse_block();
}

/*
 * Block ::= '{' Statement* '}'
 */
void Parser::parse_block() {
    // '{'
    mLexer->expect_token(TOK_BRACE_LEFT);
    print_current_token();

    // TODO: Blocks need to parse multiple statements
    // Statement*
    parse_statement();

    // '}'
    mLexer->expect_token(TOK_BRACE_RIGHT);
    print_current_token();
}

/*
 * Statement ::= BasicStatement ';'
 */
void Parser::parse_statement() {
    // BasicStatement
    parse_basic_statement();

    // ';'
    mLexer->expect_token(TOK_SEMICOLON);
    print_current_token();
}

 /*
  * BasicStatement ::=
  *      ReturnStatement
  */
void Parser::parse_basic_statement() {
    parse_return_statement();
}

/*
 * ReturnStatement ::= "return" Expression
 */
void Parser::parse_return_statement() {
    mLexer->expect_token(TOK_KEYWORD_RETURN);
    print_current_token();
    parse_expression();
}

/*
 * Expression ::=
 *      | IntegerLiteral
 *      | FloatLiteral
 */
void Parser::parse_expression() {
    mLexer->next_token();
    if (mLexer->is_int()) {
        int i = std::get<Lexer::vIntLiteral>(mLexer->mCurrentValue);
        std::cout << "DEBUG: Lexer consumed int: " << i << '\n';
    } else if (mLexer->is_float()) {
        float f = std::get<Lexer::vFloatLiteral>(mLexer->mCurrentValue);
        std::cout << "DEBUG: Lexer consumed float: " << f << '\n';
    } else {
        std::cout << "DEBUG: Lexer did not consume experssion\n"; 
    }
    print_current_token();
}

void Parser::print_current_token() {
    std::cout << "Token line number: "   << mLexer->mCurrentToken.line       << '\n'
              << "Token starts at pos: " << mLexer->mCurrentToken.start_pos  << '\n'
              << "Token has kind: "      << mLexer->mCurrentToken.kind       << '\n'
              << "====="                                                     << '\n';
}

} // namesapce charlie
