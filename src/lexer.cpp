#include "lexer.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace charlie {

static Token MakeToken(uint32_t line_start,
                       uint32_t line_end,
                       uint32_t pos_start,
                       uint32_t pos_end,
                       TokenKind kind,
                       TokenValue value) {
  Span span {line_start, line_end, pos_start, pos_end};
  return {span, kind, value};
}

static Token ErrorToken(uint32_t line_start,
                        uint32_t line_end,
                        uint32_t pos_start,
                        uint32_t pos_end) {
  return MakeToken(
    line_start, line_end, pos_start, pos_end, TOK_ERROR, TokenValue());
}

static Token DefaultToken() {
  return {{}, TOK_NO_VALUE, TokenValue()};
}

const char *GetTokenName(TokenKind kind) {
  const char *name = "";
  switch (kind) {
  case TOK_KEYWORD_USE: name = "use"; break;
  case TOK_KEYWORD_PROC: name = "proc"; break;
  case TOK_KEYWORD_LET: name = "let"; break;
  case TOK_KEYWORD_FOR: name = "for"; break;
  case TOK_KEYWORD_WHILE: name = "while"; break;
  case TOK_KEYWORD_IF: name = "if"; break;
  case TOK_KEYWORD_ELSE: name = "else"; break;
  case TOK_KEYWORD_STRUCT: name = "struct"; break;
  case TOK_KEYWORD_ENUM: name = "enum"; break;
  case TOK_KEYWORD_RETURN: name = "return"; break;

  case TOK_STRING: name = "string"; break;
  case TOK_RAW_STRING: name = "raw string"; break;
  case TOK_INT_LITERAL: name = "int literal"; break;
  case TOK_FLOAT_LITERAL: name = "float literal"; break;
  case TOK_IDENTIFIER: name = "identifier"; break;

  case TOK_OP_PLUS: name = "+"; break;
  case TOK_OP_MINUS: name = "-"; break;
  case TOK_OP_MUL: name = "*"; break;
  case TOK_OP_DIV: name = "/"; break;
  case TOK_OP_MODULO: name = "%"; break;
  case TOK_OP_GT: name = ">"; break;
  case TOK_OP_LT: name = "<"; break;
  case TOK_OP_EQ: name = "="; break;

  case TOK_COMMA: name = ","; break;
  case TOK_EQUAL: name = "="; break;
  case TOK_SEMICOLON: name = ";"; break;
  case TOK_COLON: name = ":"; break;
  case TOK_DOT: name = "."; break;
  case TOK_PAREN_LEFT: name = "("; break;
  case TOK_PAREN_RIGHT: name = ")"; break;
  case TOK_BRACKET_LEFT: name = "["; break;
  case TOK_BRACKET_RIGHT: name = "]"; break;
  case TOK_BRACE_LEFT: name = "{"; break;
  case TOK_BRACE_RIGHT: name = "}"; break;
  case TOK_DASH: name = "-"; break;

  default: break;
  }
  return name;
}

Lexer::Lexer(const std::string &file) :
    mFileStream(std::ifstream(file)), mLine(1), mPos(0),
    mLastToken(DefaultToken()) {}

Lexer::~Lexer() {}

void Lexer::GetNextToken(Token &tok) {
  if (bool success = Advance(tok, mLine, mPos); !success && tok.kind != TOK_EOF) {
    fprintf(stderr, "[Lexer Error] <%d,%d>: Failed to lex! \n", tok.span.line_start, tok.span.pos_start);
  }
  mLine = tok.span.line_end;
  mPos = tok.span.pos_end;
  mLastToken = tok;
}

void Lexer::PeekNextToken(Token &tok) {
  int file_pos_start = mFileStream.tellg();
  if (bool success = Advance(tok, mLine, mPos); !success && tok.kind != TOK_EOF) {
    fprintf(stderr, "[Lexer Error] <%d,%d>: Failed to lex! \n", tok.span.line_start, tok.span.pos_start);
  }
  mFileStream.seekg(file_pos_start);
}

void Lexer::SkipWhitespace(uint32_t *line, uint32_t *pos) {
  char c;
  while ((c = mFileStream.get()) && isspace(c)) {
    if (c == '\n') {
      (*line)++;
      *pos = 0;
    } else {
      (*pos)++;
    }
  }

  if (mFileStream.eof())
    return;

  mFileStream.unget();
}

bool Lexer::Expect(TokenKind kind, Token &tok) {
  tok = GetNextToken();
  return tok.kind == kind;
}

bool Lexer::Advance(Token &tok, uint32_t line, uint32_t pos) {
  SkipWhitespace(&line, &pos);

  bool success = false;
  char c;

  if (mFileStream.eof()) {
    tok = MakeToken(line, line, pos, pos, TOK_EOF, TokenValue());
    success = false;
    goto done;
  }

  c = mFileStream.peek();
  if (isalpha(c)) {
    success = HandleIdentifier(tok, line, pos);
  } else if (isdigit(c)) {
    // Consume float or integer

    // Try float first since a float may contain a valid integer
    // e.g. 120.02 - '120' is a valid integer
    if (success = HandleFloat(tok, line, pos); success) {
      goto done;
    }

    success = HandleInt(tok, line, pos);
  } else if (ispunct(c)) {
    if (c == '"') {
      // consume string
      success = HandleString(tok, line, pos);
    } else {
      // consume operator or punctuation
      if (success = HandlePunctuation(tok, line, pos); success) {
        goto done;
      }

      success = HandleOperator(tok, line, pos);
    }
  }

done:
  return success;
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

TokenKind Lexer::IsOperator(const char input) const noexcept {
  TokenKind tok = TOK_ERROR;
  for (int i = 0; i < kNumOps; ++i) {
    if (input == mOpMap[i].op) {
      tok = mOpMap[i].tok;
      break;
    }
  }
  return tok;
}

bool Lexer::HandleIdentifier(Token &tok, uint32_t line, uint32_t pos) {
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

  uint32_t pos_start = pos + 1;
  uint32_t pos_end = pos_start + s.length() - 1;

  tok = MakeToken(line, line, pos_start, pos_end, kind, value);

  return true;
}

bool Lexer::HandleInt(Token &tok, uint32_t line, uint32_t pos) {
  int file_pos_start = mFileStream.tellg();
  char c = mFileStream.get();
  uint32_t pos_start = ++pos;

  if (c == '0' && isdigit(mFileStream.peek())) {
    tok = ErrorToken(line, line, pos_start, pos_start);
    mFileStream.seekg(file_pos_start);
    return false;
  } else if (c == '0') {
    tok = MakeToken(line, line, pos_start, pos_start, TOK_INT_LITERAL, TokenValue(0));
    return true;
  }

  TokenValue value;
  int &i = value.emplace<int>();
  i = c - '0';
  while ((c = mFileStream.get()) && isdigit(c)) {
    i = (i * 10) + (c - '0');
    pos++;
  }
  mFileStream.unget();

  tok = MakeToken(line, line, pos_start, pos, TOK_INT_LITERAL, value);

  return true;
}

bool Lexer::HandleFloat(Token &tok, uint32_t line, uint32_t pos) {
  int file_pos_start = mFileStream.tellg();
  uint32_t pos_start = ++pos;

  char c = mFileStream.get();
  if (c == '0' && mFileStream.peek() != '.') {
    mFileStream.seekg(file_pos_start);
    return false;
  }

  std::string s {c};

  while ((c = mFileStream.get()) && isdigit(c)) {
    s += c;
  }

  if (c != '.') {
    uint32_t pos_end = pos_start + (s.length() - 1);
    tok = ErrorToken(line, line, pos_start, pos_end);
    mFileStream.seekg(file_pos_start);
    return false;
  }

  s += c;

  while ((c = mFileStream.get()) && isdigit(c)) {
    s += c;
  }
  mFileStream.unget();

  TokenValue value;
  float &f = value.emplace<float>();
  f = atof(s.c_str());
  std::cout << "DEBUG: parsed float " << f << " from string " << s << '\n';

   pos += (s.length() - 1);

  tok = MakeToken(line, line, pos_start, pos, TOK_FLOAT_LITERAL, value);

  return true;
}

bool Lexer::HandleString(Token &tok, uint32_t line, uint32_t pos) {
  int file_pos_start = mFileStream.tellg();
  uint32_t pos_start = ++pos;

  TokenValue value;
  std::string &str = value.emplace<std::string>();
  mFileStream.ignore();
  char c;
  while ((c = mFileStream.get()) && isprint(c) && c != '"') {
    str += c;
  }

  if (c != '"') {
    tok = ErrorToken(line, line, pos_start, pos_start);
    mFileStream.seekg(file_pos_start);
    return false;
  }

  pos += str.length();

  tok = MakeToken(line, line, pos_start, pos, TOK_STRING, value);

  return true;
}

bool Lexer::HandleOperator(Token &tok, uint32_t line, uint32_t pos) {
  char c = mFileStream.peek();

  TokenKind kind = IsOperator(c);
  if (kind == TOK_ERROR) {
    tok = ErrorToken(line, line, pos, pos);
    return false;
  }

  TokenValue value;
  char &punc = value.emplace<char>();
  punc = mFileStream.get();
  pos++;

  tok = MakeToken(line, line, pos, pos, kind, value);

  return true;
}

bool Lexer::HandlePunctuation(Token &tok, uint32_t line, uint32_t pos) {
  char c = mFileStream.peek();

  TokenKind kind = IsPunctuation(c);
  if (kind == TOK_ERROR) {
    tok = ErrorToken(line, line, pos, pos);
    return false;
  }

  TokenValue value;
  char &punc = value.emplace<char>();
  punc = mFileStream.get();
  pos++;

  tok = MakeToken(line, line, pos, pos, kind, value);
  return true;
}

}  // namespace charlie
