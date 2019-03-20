#pragma once

#include <cstring>
#include <fstream>
#include <variant>

namespace charlie {

enum TokenKind : uint32_t {

  // KEYWORDS
  TOK_KEYWORD_START = 100,
  TOK_KEYWORD_USE = TOK_KEYWORD_START,
  TOK_KEYWORD_FUN = 101,
  TOK_KEYWORD_LET = 102,
  TOK_KEYWORD_FOR = 103,
  TOK_KEYWORD_WHILE = 104,
  TOK_KEYWORD_IF = 105,
  TOK_KEYWORD_ELSE = 106,
  TOK_KEYWORD_STRUCT = 107,
  TOK_KEYWORD_ENUM = 108,
  TOK_KEYWORD_RETURN = 109,
  // Add keywords as they come and update TOK_KEYWORD_END
  TOK_KEYWORD_END = 110,

  TOK_STRING = 400,
  TOK_RAW_STRING = 401,
  TOK_INT_LITERAL = 402,
  TOK_FLOAT_LITERAL = 403,
  TOK_IDENTIFIER = 404,

  // OPERATORS
  TOK_OP_PLUS = 500,
  TOK_OP_MINUS = 501,
  TOK_OP_MUL = 502,
  TOK_OP_DIV = 503,
  TOK_OP_MODULO = 504,
  TOK_OP_GT = 505,
  TOK_OP_GE = 506,
  TOK_OP_LT = 507,
  TOK_OP_LE = 508,

  // PUNCTUATION
  TOK_PUNC_START = 800,
  TOK_COMMA = TOK_PUNC_START,
  TOK_EQUAL = 801,
  TOK_SEMICOLON = 802,
  TOK_COLON = 803,
  TOK_DOT = 804,
  TOK_PAREN_LEFT = 805,
  TOK_PAREN_RIGHT = 806,
  TOK_BRACKET_LEFT = 807,
  TOK_BRACKET_RIGHT = 808,
  TOK_BRACE_LEFT = 809,
  TOK_BRACE_RIGHT = 810,
  // Add punctuation as they come and update TOK_PUNC_END
  TOK_PUNC_END = 811,

  TOK_EOF = (0x0E0F'E0F0),

  TOK_ERROR = (0x7FFF'FFFF)
};

struct Span {
  uint32_t line_start;
  uint32_t line_end;
  uint32_t pos_start;
  uint32_t pos_end;
};

using TokenValue = std::variant<int, float, char, std::string>;
struct Token {
  Span span;
  TokenKind kind;
  TokenValue value;
};

class Lexer {
public:
  Lexer(const std::string &file);
  ~Lexer();

  // Sets |tok| to the next token.
  //
  // If there was en error in lexing then |tok| will be set to a token with
  // kind TOK_ERROR and have a span pointing to the location of the token
  void GetNextToken(Token &tok);
  Token GetNextToken();

  // Gets the next token and compares its kind with |kind|.
  // |tok| is set to the next token.
  bool Expect(TokenKind kind, Token &tok);

private:
  constexpr static size_t kNumKeywords = TOK_KEYWORD_END - TOK_KEYWORD_START;
  constexpr static struct {
    const char *kw;
    TokenKind tok;
  } mKeywordMap[kNumKeywords] = {
    {"if", TOK_KEYWORD_IF},
    {"use", TOK_KEYWORD_USE},
    {"fun", TOK_KEYWORD_FUN},
    {"let", TOK_KEYWORD_LET},
    {"for", TOK_KEYWORD_FOR},
    {"else", TOK_KEYWORD_ELSE},
    {"enum", TOK_KEYWORD_ENUM},
    {"while", TOK_KEYWORD_WHILE},
    {"struct", TOK_KEYWORD_STRUCT},
    {"return", TOK_KEYWORD_RETURN},
  };

  constexpr static size_t kNumPunc = TOK_PUNC_END - TOK_PUNC_START;
  constexpr static struct {
    const char punc;
    TokenKind tok;
  } mPuncMap[kNumPunc] = {
    {',', TOK_COMMA},
    {'=', TOK_EQUAL},
    {';', TOK_SEMICOLON},
    {':', TOK_COLON},
    {'.', TOK_DOT},
    {'(', TOK_PAREN_LEFT},
    {')', TOK_PAREN_RIGHT},
    {'[', TOK_BRACKET_LEFT},
    {']', TOK_BRACKET_RIGHT},
    {'{', TOK_BRACE_LEFT},
    {'}', TOK_BRACE_RIGHT},
  };

  std::ifstream mFileStream;

  uint32_t mLine;
  uint32_t mPos;

  // Skip whitespaces where next character to be read is the first
  // non-whitespace character
  void SkipWhitespace();

  TokenKind IsKeyword(const char *input) const noexcept;
  TokenKind IsPunctuation(const char input) const noexcept;

  // Handle an identifier.
  // This also includes checking to see if the identifier is also a keyword
  bool HandleIdentifier(Token &tok);
  bool HandleFloat(Token &tok);
  bool HandleInt(Token &tok);
  bool HandleString(Token &tok);
  bool HandleOperator(Token &tok);
  bool HandlePunctuation(Token &tok);

};  // class Lexer

}  // namespace charlie
