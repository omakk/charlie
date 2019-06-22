#include "lexer.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace charlie {

static Token ErrorToken(uint32_t line_start,
                        uint32_t line_end,
                        uint32_t pos_start,
                        uint32_t pos_end) {
  Span span {line_start, line_end, pos_start, pos_end};
  return {span, TOK_ERROR, TokenValue()};
}

Lexer::Lexer(const std::string &file) :
    mFileStream(std::ifstream(file)), mLine(1), mPos(0) {}

Lexer::~Lexer() {}

void Lexer::GetNextToken(Token &tok) {
  SkipWhitespace();

  if (mFileStream.eof()) {
    Span span {mLine, mLine, mPos, mPos};
    tok = {span, TOK_EOF, TokenValue()};
  }

  bool success = false;
  char c = mFileStream.peek();
  if (isalpha(c)) {
    success = HandleIdentifier(tok);
  } else if (isdigit(c)) {
    // Consume float or integer

    // Try float first since a float may contain a valid integer
    // e.g. 120.02 - '120' is a valid integer
    success = HandleFloat(tok);
    if (success)
      return;

    success = HandleInt(tok);
  } else if (ispunct(c)) {
    if (c == '"') {
      // consume string
      // handle_string();
    } else {
      // consume operator or punctuation
      success = HandlePunctuation(tok);
    }
  }

  if (!success) {
    fprintf(stderr, "[Lexer Error] <%d,%d>: Failed to lex! \n", mLine, mPos);
  }
}

Token Lexer::GetNextToken() {
  Token tok;
  GetNextToken(tok);
  return tok;
}

void Lexer::SkipWhitespace() {
  char c;
  while ((c = mFileStream.get()) && isspace(c)) {
    if (c == '\n') {
      mLine++;
      mPos = 0;
    } else {
      mPos++;
    }
  }
  mFileStream.unget();
}

bool Lexer::Expect(TokenKind kind, Token &tok) {
  tok = GetNextToken();
  return tok.kind == kind;
}

TokenKind Lexer::IsKeyword(const char *input) const noexcept {
  TokenKind tok = TOK_ERROR;
  for (int i = 0; i < kNumKeywords; ++i) {
    if (!strcmp(input, mKeywordMap[i].kw)) {
      tok = mKeywordMap[i].tok;
      break;
    }
  }
  return tok;
}

TokenKind Lexer::IsPunctuation(const char input) const noexcept {
  TokenKind tok = TOK_ERROR;
  for (int i = 0; i < kNumPunc; ++i) {
    if (input == mPuncMap[i].punc) {
      tok = mPuncMap[i].tok;
      break;
    }
  }
  return tok;
}

bool Lexer::HandleIdentifier(Token &tok) {
  char c;
  TokenValue value;
  std::string &s = value.emplace<std::string>();

  while ((c = mFileStream.get())) {
    if (isspace(c) || !(isalnum(c) || c == '_')) {
      mFileStream.unget();
      break;
    }
    s += c;
  }

  TokenKind kind = IsKeyword(s.c_str());
  if (kind == TOK_ERROR) {
    kind = TOK_IDENTIFIER;
  }

  uint32_t pos_start = mPos + 1;
  mPos += s.length();

  Span span {mLine, mLine, pos_start, mPos};
  tok = {span, kind, value};

  return true;
}

bool Lexer::HandleInt(Token &tok) {
  char c = mFileStream.get();
  uint32_t pos_start = mPos + 1;

  if (c == '0' && isdigit(mFileStream.peek())) {
    tok = ErrorToken(mLine, mLine, pos_start, pos_start);
    mFileStream.unget();
    return false;
  } else if (c == '0') {
    Span span {mLine, mLine, pos_start, pos_start};
    tok = {span, TOK_INT_LITERAL, TokenValue(0)};
    mPos++;
    return true;
  }

  TokenValue value;
  int &i = value.emplace<int>();
  i = c - '0';
  int pos = 1;
  while ((c = mFileStream.get()) && isdigit(c)) {
    i = (i * 10) + (c - '0');
    pos++;
  }

  mFileStream.unget();

  mPos += pos;

  Span span {mLine, mLine, pos_start, mPos};
  tok = {span, TOK_INT_LITERAL, value};

  return true;
}

bool Lexer::HandleFloat(Token &tok) {
  std::string s;
  char c = mFileStream.get();
  uint32_t pos_start = mPos + 1;

  if (c == '0' && mFileStream.peek() != '.') {
    mFileStream.unget();
    return false;
  }

  s += c;

  while ((c = mFileStream.get()) && isdigit(c)) {
    s += c;
  }

  if (c != '.') {
    uint32_t pos_end = pos_start + (s.length() - 1);
    tok = ErrorToken(mLine, mLine, pos_start, pos_end);
    // We should be peeking at the first character that we got
    for (int i = 0; i < s.length() + 1; ++i)
      mFileStream.unget();
    return false;
  }

  s += c;

  while ((c = mFileStream.get()) && isdigit(c)) {
    s += c;
  }

  TokenValue value;
  float &f = value.emplace<float>();
  f = atof(s.c_str());
  std::cout << "DEBUG: parsed float " << f << " from string " << s << '\n';

  mFileStream.unget();

  mPos += s.length();

  Span span {mLine, mLine, pos_start, mPos};
  tok = {span, TOK_FLOAT_LITERAL, value};

  return true;
}

bool Lexer::HandleString(Token &tok) {
  return false;
}

bool Lexer::HandleOperator(Token &tok) {
  return false;
}

bool Lexer::HandlePunctuation(Token &tok) {
  char c = mFileStream.peek();

  TokenKind kind = IsPunctuation(c);
  if (kind == TOK_ERROR) {
    tok = ErrorToken(mLine, mLine, mPos, mPos);
    return false;
  }

  TokenValue value;
  char &punc = value.emplace<char>();
  punc = mFileStream.get();
  mPos++;

  Span span {mLine, mLine, mPos, mPos};
  tok = {span, kind, value};

  return true;
}

}  // namespace charlie
