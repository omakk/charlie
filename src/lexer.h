#pragma once

#include <cstring>
#include <fstream>
#include <variant>

namespace charlie {

enum TokenKind : uint32_t {
  TOK_NO_VALUE = 0,

  // KEYWORDS
  TOK_KEYWORD_START = 100,
  TOK_KEYWORD_USE = TOK_KEYWORD_START,
  TOK_KEYWORD_PROC = 101,
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
  TOK_OP_START = 500,
  TOK_OP_PLUS = TOK_OP_START,
  TOK_OP_MINUS = 501,
  TOK_OP_MUL = 502,
  TOK_OP_DIV = 503,
  TOK_OP_MODULO = 504,
  TOK_OP_GT = 505,
  TOK_OP_LT = 506,
  TOK_OP_EQ = 507,
  // Add operators as they come and update TOK_OP_END
  TOK_OP_END = 508,

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
  TOK_DASH = 811,
  // Add punctuation as they come and update TOK_PUNC_END
  TOK_PUNC_END = 812,

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

const char *GetTokenName(TokenKind kind);

class Lexer {
public:
  Lexer(const std::string &file);
  ~Lexer();

  // Sets `tok` to the next token and advances the lexer.
  //
  // If an error occured during lexing then `tok` will have kind TOK_ERROR.
  // If we reached EOF then `tok` will have kind TOK_EOF.
  void GetNextToken(Token &tok);
  Token GetNextToken() { Token t; GetNextToken(t); return t;}

  // Sets `tok` to the next token but does not advance the lexer.
  //
  // If an error occured during lexing then `tok` will have kind TOK_ERROR.
  // If we reached EOF then `tok` will have kind TOK_EOF.
  void PeekNextToken(Token &tok);
  Token PeekNextToken() { Token t; PeekNextToken(t); return t;}

  // Sets `tok` to the last successfully lexed token.
  void GetToken(Token &tok) { tok = mLastToken; }
  // Returns the last successfully lexed token
  Token GetToken() { return mLastToken; }

  // Gets the next token and compares its kind with |kind|.
  // |tok| is set to the next token.
  bool Expect(TokenKind kind, Token &tok);

private:
  constexpr static int kNumKeywords = TOK_KEYWORD_END - TOK_KEYWORD_START;
  constexpr static struct {
    const char *kw;
    TokenKind tok;
  } mKeywordMap[kNumKeywords] = {
    {"if", TOK_KEYWORD_IF},
    {"use", TOK_KEYWORD_USE},
    {"let", TOK_KEYWORD_LET},
    {"for", TOK_KEYWORD_FOR},
    {"proc", TOK_KEYWORD_PROC},
    {"else", TOK_KEYWORD_ELSE},
    {"enum", TOK_KEYWORD_ENUM},
    {"while", TOK_KEYWORD_WHILE},
    {"struct", TOK_KEYWORD_STRUCT},
    {"return", TOK_KEYWORD_RETURN},
  };

  constexpr static int kNumPunc = TOK_PUNC_END - TOK_PUNC_START;
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
    {'-', TOK_DASH},
  };

  constexpr static int kNumOps = TOK_OP_END - TOK_OP_START;
  constexpr static struct {
    const char op;
    TokenKind tok;
  } mOpMap[kNumOps] = {
    {'+', TOK_OP_PLUS},
    {'-', TOK_OP_MINUS},
    {'*', TOK_OP_MUL},
    {'/', TOK_OP_DIV},
    {'%', TOK_OP_MODULO},
    {'>', TOK_OP_GT},
    {'<', TOK_OP_LT},
    {'=', TOK_OP_EQ},
  };

  std::ifstream mFileStream;

  uint32_t mLine;
  uint32_t mPos;

  Token mLastToken;

  // Advances the lexer forward one token and assigns it to `tok`
  bool Advance(Token &tok, uint32_t line, uint32_t pos);

  // Skip whitespaces where next character to be read is the first
  // non-whitespace character
  void SkipWhitespace(uint32_t *line, uint32_t *pos);

  TokenKind IsKeyword(const char *input) const noexcept;
  TokenKind IsPunctuation(const char input) const noexcept;
  TokenKind IsOperator(const char input) const noexcept;

  // Handle an identifier.
  // This also includes checking to see if the identifier is also a keyword
  bool HandleIdentifier(Token &tok, uint32_t line, uint32_t pos);
  bool HandleFloat(Token &tok, uint32_t line, uint32_t pos);
  bool HandleInt(Token &tok, uint32_t line, uint32_t pos);
  bool HandleString(Token &tok, uint32_t line, uint32_t pos);
  bool HandleOperator(Token &tok, uint32_t line, uint32_t pos);
  bool HandlePunctuation(Token &tok, uint32_t line, uint32_t pos);

};  // class Lexer

}  // namespace charlie
