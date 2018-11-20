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
    parse_function_definition();
}

void Parser::parse_function_definition() {
    mLexer->expect_token(TOK_KEYWORD_FUN);
    print_current_token();
    mLexer->next_token();
    if(mLexer->is_identifier()) {
        std::string &s = std::get<Lexer::vIdentifier>(mLexer->mCurrentValue);
        std::cout << "DEBUG: Lexer consumed identifier: " << s << '\n';
    }
    print_current_token();
    mLexer->expect_token(TOK_PAREN_LEFT);
    print_current_token();
    // parse_function_parameters();
    mLexer->expect_token(TOK_PAREN_RIGHT);
    print_current_token();
    parse_block();
}

void Parser::parse_block() {
    mLexer->expect_token(TOK_BRACE_LEFT);
    print_current_token();
    // TODO: Blocks need to parse multiple statements
    parse_statement();
    mLexer->expect_token(TOK_BRACE_RIGHT);
    print_current_token();
}

void Parser::parse_statement() {
    parse_basic_statement();
    mLexer->expect_token(TOK_SEMICOLON);
    print_current_token();
}

void Parser::parse_basic_statement() {
    parse_return_statement();
}

void Parser::parse_return_statement() {
    mLexer->expect_token(TOK_KEYWORD_RETURN);
    print_current_token();
    parse_expression();
}

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
