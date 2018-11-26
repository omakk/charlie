#pragma once

namespace Charlie {

namespace Lexer {

enum Token : uint64_t {

    // KEYWORDS
    TOK_KEYWORD_USE,
    TOK_KEYWORD_FUN,
    TOK_KEYWORD_LET,
    TOK_KEYWORD_FOR,
    TOK_KEYWORD_WHILE,
    TOK_KEYWORD_IF,
    TOK_KEYWORD_ELSE,
    TOK_KEYWORD_STRUCT,
    TOK_KEYWORD_ENUM,

    TOK_STRING,
    TOK_RAW_STRING,
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_IDENTIFIER,

    // OPERATORS
    TOK_OP_PLUS,
    TOK_OP_MINUS,
    TOK_OP_MUL,
    TOK_OP_DIV,
    TOK_OP_MODULO,
    TOK_OP_GT,
    TOK_OP_GE,
    TOK_OP_LT,
    TOK_OP_LE,

    // PUNCTUATION
    TOK_COMMA,
    TOK_EQUAL,
    TOK_SEMICOLON,
    TOK_COLON,
    TOK_DOT,
    TOK_ARROW,
    TOK_PAREN_LEFT,
    TOK_PAREN_RIGHT,
    TOK_BRACKET_LEFT,
    TOK_BRACKET_RIGHT,
    TOK_BRACE_LEFT,
    TOK_BRACE_RIGHT,

    TOK_EOF,
};

class Tokenizer {
public:
    Tokenizer();
    ~Tokenizer();

};

} // namespace Lexer
} // namespace Charlie
