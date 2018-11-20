#pragma once

#include <fstream>
#include <map>
#include <variant>

namespace charlie {

enum TokenKind : uint32_t {

    // KEYWORDS
    TOK_KEYWORD_USE    = 100,
    TOK_KEYWORD_FUN    = 101,
    TOK_KEYWORD_LET    = 102,
    TOK_KEYWORD_FOR    = 103,
    TOK_KEYWORD_WHILE  = 104,
    TOK_KEYWORD_IF     = 105,
    TOK_KEYWORD_ELSE   = 106,
    TOK_KEYWORD_STRUCT = 107,
    TOK_KEYWORD_ENUM   = 108,
    TOK_KEYWORD_RETURN = 109,

    TOK_STRING         = 400,
    TOK_RAW_STRING     = 401,
    TOK_INT_LITERAL    = 402,
    TOK_FLOAT_LITERAL  = 403,
    TOK_IDENTIFIER     = 404,

    // OPERATORS
    TOK_OP_PLUS        = 500,
    TOK_OP_MINUS       = 501,
    TOK_OP_MUL         = 502,
    TOK_OP_DIV         = 503,
    TOK_OP_MODULO      = 504,
    TOK_OP_GT          = 505,
    TOK_OP_GE          = 506,
    TOK_OP_LT          = 507,
    TOK_OP_LE          = 508,

    // PUNCTUATION
    TOK_COMMA          = 800,
    TOK_EQUAL          = 801,
    TOK_SEMICOLON      = 802,
    TOK_COLON          = 803,
    TOK_DOT            = 804,
    TOK_PAREN_LEFT     = 805,
    TOK_PAREN_RIGHT    = 806,
    TOK_BRACKET_LEFT   = 807,
    TOK_BRACKET_RIGHT  = 808,
    TOK_BRACE_LEFT     = 809,
    TOK_BRACE_RIGHT    = 810,

    TOK_EOF            = (0x0E0F'E0F0),

    TOK_ERROR          = (0x7FFF'FFFF)
};

struct Token {
    uint32_t line;
    uint32_t start_pos;
    TokenKind kind;
    //TokenValue value;
};

class Lexer {
public:
    static const std::map<const std::string, TokenKind> mKeywordsMap;
    static const std::map<const char, TokenKind> mPunctuationMap;

    enum VariantValueIndex : uint8_t {
        vIdentifier = 0,
        vKeyword,
        vStringLiteral,
        vIntLiteral,
        vFloatLiteral,
        vPunctuation,
        vOp
    };
    std::variant<std::string, std::string, std::string,
                 int, float, const char, const char> mCurrentValue;

    std::ifstream mFileStream;
    std::string mFilename;

    Token mCurrentToken;
    uint32_t mCurrentLine;
    uint32_t mCurrentPos;

    Lexer(const std::string& file);
    ~Lexer();

    void next_token();

    inline bool is_identifier()  { return mCurrentToken.kind == TOK_IDENTIFIER; }
    inline bool is_int()         { return mCurrentToken.kind == TOK_INT_LITERAL; }
    inline bool is_float()       { return mCurrentToken.kind == TOK_FLOAT_LITERAL; }
    inline bool is_string()      { return false; }
    inline bool is_operator()    { return false; }
    inline bool is_punctuation() { return false; }


    void expect_token(TokenKind kind);

private:

    void skip_whitespace();

    TokenKind is_keyword(std::string& s);

    bool handle_keyword_or_identifier();
    bool handle_float();
    bool handle_int();
    bool handle_string();
    bool handle_operator();
    bool handle_punctuation();

}; // class Lexer

void fail(const char* msg, ...);

} // namespace charlie
