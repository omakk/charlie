#include "lexer.h"

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace charlie {

const std::map<const std::string, TokenKind> Lexer::mKeywordsMap = {
  {"fun", TOK_KEYWORD_FUN}, {"return", TOK_KEYWORD_RETURN}};

const std::map<const char, TokenKind> Lexer::mPunctuationMap = {
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

Lexer::Lexer(const std::string &file) :
    mFileStream(std::ifstream(file)), mFilename(file), mCurrentLine(1),
    mCurrentPos(1) {}

Lexer::~Lexer() {}

void Lexer::next_token() {
  skip_whitespace();
  char c                  = mFileStream.peek();
  bool next_token_success = false;

  if (mFileStream.eof()) {
    mCurrentToken = {mCurrentLine, mCurrentPos, TOK_EOF};
    return;
  }

  if (isalpha(c)) {
    next_token_success = handle_keyword_or_identifier();
  } else if (isdigit(c)) {
    // consume float or integer

    // Try float first since a float may contain a valid integer
    // e.g. 120.02 - '120' is a valid integer
    next_token_success = handle_float();
    if (next_token_success)
      return;

    next_token_success = handle_int();
  } else if (ispunct(c)) {
    if (c == '"') {
      // consume string
      // handle_string();
    } else {
      // consume operator or punctuation
      next_token_success = handle_punctuation();
    }
  } else {
    fail("[Lexer Error] %s <%d,%d>: Unexpected '%c' \n",
         mFilename.c_str(),
         mCurrentLine,
         mCurrentPos,
         c);
  }

  if (!next_token_success) {
    fail("[Lexer Error] %s <%d,%d>: Failed to lex \n",
         mFilename.c_str(),
         mCurrentLine,
         mCurrentPos);
  }
}

void Lexer::skip_whitespace() {
  char c;
  while ((c = mFileStream.peek())) {
    if (!isspace(c)) {
      break;
    }
    if (c == '\n') {
      mCurrentLine++;
      mCurrentPos = 1;
    } else {
      mCurrentPos++;
    }
    mFileStream.get();
  }
}

void Lexer::expect_token(TokenKind kind) {
  next_token();
  if (mCurrentToken.kind != kind) {
    fail("[Lexer Error] %s<%d,%d> :: Unexpected token\n",
         mFilename.c_str(),
         mCurrentLine,
         mCurrentToken.start_pos);
  }
}

TokenKind Lexer::is_keyword(std::string &s) {
  std::map<const std::string, TokenKind>::const_iterator it =
    mKeywordsMap.find(s);
  if (it == mKeywordsMap.end())
    return TOK_ERROR;
  return it->second;
}

bool Lexer::handle_keyword_or_identifier() {
  char c;
  std::string s;
  TokenKind k = TOK_ERROR;

  while ((c = mFileStream.get())) {
    if (isspace(c) || !(isalnum(c) || c == '_')) {
      mFileStream.unget();
      break;
    }
    s += c;
  }

  k = is_keyword(s);
  if (k == TOK_ERROR) {
    k = TOK_IDENTIFIER;
  }

  uint32_t start_pos = mCurrentPos;
  mCurrentPos += s.length();

  if (k == TOK_IDENTIFIER) {
    mCurrentValue.emplace<vIdentifier>(std::move(s));
  } else {
    mCurrentValue.emplace<vKeyword>(std::move(s));
  }

  mCurrentToken = {mCurrentLine, start_pos, k};

  return true;
}

bool Lexer::handle_int() {
  char c;
  uint32_t start_pos = mCurrentPos;
  uint32_t pos       = 0;
  int integer_value  = 0;

  while ((c = mFileStream.get()) && isdigit(c)) {
    if (pos == 0 && c == '0') {
      if (isdigit(mFileStream.peek())) {
        mCurrentToken = {mCurrentLine, start_pos, TOK_ERROR};
        mFileStream.unget();
        return false;
      } else {
        mCurrentValue.emplace<vIntLiteral>(0);
        mCurrentToken = {mCurrentLine, start_pos, TOK_INT_LITERAL};
        return true;
      }
    }

    integer_value = (integer_value * 10) + (c - '0');
    pos++;
  }

  mFileStream.unget();
  mCurrentValue.emplace<vIntLiteral>(integer_value);
  mCurrentPos += pos;
  mCurrentToken = {mCurrentLine, start_pos, TOK_INT_LITERAL};

  return true;
}

bool Lexer::handle_float() {
  std::string s;
  char c             = mFileStream.get();
  uint32_t start_pos = mCurrentPos;

  if (c == '0' && mFileStream.peek() != '.') {
    mCurrentToken = {mCurrentLine, start_pos, TOK_ERROR};
    mFileStream.unget();
    return false;
  }

  s += c;

  while ((c = mFileStream.get()) && isdigit(c)) {
    s += c;
  }

  if (c != '.') {
    mCurrentToken = {mCurrentLine, start_pos, TOK_ERROR};
    // We should be peeking at the first character that we got
    for (int i = 0; i < s.length() + 1; ++i)
      mFileStream.unget();
    return false;
  }

  s += c;

  while ((c = mFileStream.get()) && isdigit(c)) {
    s += c;
  }

  std::cout << "DEBUG: parsed float string " << s << '\n';
  float f = atof(s.c_str());
  std::cout << "DEBUG: parsed float " << f << '\n';
  mFileStream.unget();
  mCurrentValue.emplace<vFloatLiteral>(f);
  mCurrentToken = {mCurrentLine, start_pos, TOK_FLOAT_LITERAL};
  mCurrentPos += s.length();

  return true;
}
bool Lexer::handle_string() {
  return false;
}
bool Lexer::handle_operator() {
  return false;
}
bool Lexer::handle_punctuation() {
  char c                                             = mFileStream.peek();
  std::map<const char, TokenKind>::const_iterator it = mPunctuationMap.find(c);
  if (it == mPunctuationMap.end())
    return false;

  mFileStream.ignore();  // Skip a character

  uint32_t start_pos = mCurrentPos;
  mCurrentPos++;
  mCurrentValue.emplace<vPunctuation>(it->first);
  mCurrentToken = {mCurrentLine, start_pos, it->second};

  return true;
}

void fail(const char *format_msg, ...) {
  va_list args;
  va_start(args, format_msg);
  vfprintf(stderr, format_msg, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

}  // namespace charlie
