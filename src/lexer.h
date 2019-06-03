#pragma once

#include <cstring>
#include <fstream>
#include <map>
#include <variant>

namespace charlie {

enum TokenKind : uint32_t {

  // KEYWORDS
  TOK_KEYWORD_START  = 100,
  TOK_KEYWORD_USE    = TOK_KEYWORD_START,
  TOK_KEYWORD_FUN    = 101,
  TOK_KEYWORD_LET    = 102,
  TOK_KEYWORD_FOR    = 103,
  TOK_KEYWORD_WHILE  = 104,
  TOK_KEYWORD_IF     = 105,
  TOK_KEYWORD_ELSE   = 106,
  TOK_KEYWORD_STRUCT = 107,
  TOK_KEYWORD_ENUM   = 108,
  TOK_KEYWORD_RETURN = 109,
  // Add keywords as they come and update TOK_KEYWORD_END
  TOK_KEYWORD_END = 110,

  TOK_STRING        = 400,
  TOK_RAW_STRING    = 401,
  TOK_INT_LITERAL   = 402,
  TOK_FLOAT_LITERAL = 403,
  TOK_IDENTIFIER    = 404,

  // OPERATORS
  TOK_OP_PLUS   = 500,
  TOK_OP_MINUS  = 501,
  TOK_OP_MUL    = 502,
  TOK_OP_DIV    = 503,
  TOK_OP_MODULO = 504,
  TOK_OP_GT     = 505,
  TOK_OP_GE     = 506,
  TOK_OP_LT     = 507,
  TOK_OP_LE     = 508,

  // PUNCTUATION
  TOK_PUNC_START    = 800,
  TOK_COMMA         = TOK_PUNC_START,
  TOK_EQUAL         = 801,
  TOK_SEMICOLON     = 802,
  TOK_COLON         = 803,
  TOK_DOT           = 804,
  TOK_PAREN_LEFT    = 805,
  TOK_PAREN_RIGHT   = 806,
  TOK_BRACKET_LEFT  = 807,
  TOK_BRACKET_RIGHT = 808,
  TOK_BRACE_LEFT    = 809,
  TOK_BRACE_RIGHT   = 810,
  // Add punctuation as they come and update TOK_PUNC_END
  TOK_PUNC_END = 811,

  TOK_EOF = (0x0E0F'E0F0),

  TOK_ERROR = (0x7FFF'FFFF)
};

struct Token {
  uint32_t line;
  uint32_t start_pos;
  TokenKind kind;
  // TokenValue value;
};

class Lexer {
public:
  enum VariantValueIndex : uint8_t {
    vIdentifier = 0,
    vKeyword,
    vStringLiteral,
    vIntLiteral,
    vFloatLiteral,
    vPunctuation,
    vOp
  };
  std::variant<std::string,
               std::string,
               std::string,
               int,
               float,
               const char,
               const char>
    mCurrentValue;

  std::ifstream mFileStream;
  std::string mFilename;

  Token mCurrentToken;
  uint32_t mCurrentLine;
  uint32_t mCurrentPos;

  Lexer(const std::string &file);
  ~Lexer();

  void next_token();

  inline bool is_identifier() {
    return mCurrentToken.kind == TOK_IDENTIFIER;
  }

  inline bool is_int() {
    return mCurrentToken.kind == TOK_INT_LITERAL;
  }

  inline bool is_float() {
    return mCurrentToken.kind == TOK_FLOAT_LITERAL;
  }

  inline bool is_string() {
    return false;
  }

  inline bool is_operator() {
    return false;
  }

  inline bool is_punctuation() {
    return false;
  }

  void expect_token(TokenKind kind);

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

  void skip_whitespace();

  constexpr TokenKind is_keyword(const char *input) const noexcept {
    TokenKind tok = TOK_ERROR;
    for (int i = 0; i < kNumKeywords; ++i) {
      if (!strcmp(input, mKeywordMap[i].kw)) {
        tok = mKeywordMap[i].tok;
        break;
      }
    }
    return tok;
  };

  constexpr TokenKind is_punc(const char input) const noexcept {
    TokenKind tok = TOK_ERROR;
    for (int i = 0; i < kNumPunc; ++i) {
      if (input == mPuncMap[i].punc) {
        tok = mPuncMap[i].tok;
        break;
      }
    }
    return tok;
  };

  bool handle_keyword_or_identifier();
  bool handle_float();
  bool handle_int();
  bool handle_string();
  bool handle_operator();
  bool handle_punctuation();

};  // class Lexer

void fail(const char *msg, ...);

}  // namespace charlie
