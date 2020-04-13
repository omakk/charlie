#include "parser.h"

#include <cstdarg>
#include <iostream>
#include <variant>

namespace charlie {

static void warn(const char *format_msg, ...) {
  va_list args;
  va_start(args, format_msg);
  vfprintf(stderr, format_msg, args);
  va_end(args);
}

static void print_tok(const Token &tok) {
  std::cout << "Token: Kind " << tok.kind
            << " Span(Line:  " << tok.span.line_start << " -> "
            << tok.span.line_end << ", Pos: " << tok.span.pos_start << " -> "
            << tok.span.pos_end << ") \"" << GetTokenName(tok.kind) << "\"\n";
}

Parser::Parser(std::string file) :
    mFileName(std::move(file)), mLexer(mFileName) {}

Parser::~Parser() {}

std::unique_ptr<Module> Parser::Parse() {
  std::vector<std::unique_ptr<TopLevelDeclaration>> decls;
  for (auto decl = ParseTopLevelDeclaration(); decl != nullptr;
       decl = ParseTopLevelDeclaration()) {
    decls.push_back(std::move(decl));
  }
  return std::make_unique<Module>(std::string(mFileName), std::move(decls));
}

/*
 * TopLevelDeclaration ::= ProcedureDefinition | StructDefinition
 */
std::unique_ptr<TopLevelDeclaration> Parser::ParseTopLevelDeclaration() {
  Token tok;
  bool res = mLexer.Expect(TOK_IDENTIFIER, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected identifier\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }

  std::string ident = std::get<std::string>(tok.value);
  std::cout << "DEBUG: Lexer consumed identifier: " << ident << '\n';
  print_tok(tok);

  res = mLexer.Expect(TOK_COLON, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected ':'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }

  res = mLexer.Expect(TOK_COLON, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected ':'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }

  mLexer.GetNextToken(tok);
  if (tok.kind == TOK_ERROR) {
    return nullptr;
  }

  switch (tok.kind) {
  case TOK_KEYWORD_PROC:
    print_tok(tok);
    return ParseProcedureDefintion(std::move(ident));
  case TOK_KEYWORD_STRUCT:
    print_tok(tok);
    return ParseStructDefinition(std::move(ident));
  default:
    // TODO: report some approriate warning/error
    return nullptr;
  }
}

/*
* ProcedurePrototype ::= IDENTIFIER "::" "proc" "(" { ProcdeureParameters } ")" [ "->" IDENTIFIER ]
*/
std::unique_ptr<ProcedurePrototype> Parser::ParseProcedurePrototype(std::string proc_name) {
  Token tok;

  // '('
  bool res = mLexer.Expect(TOK_PAREN_LEFT, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected '('\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  std::vector<std::string> args;
  // ParseProcedureParameters();

  // ')'
  res = mLexer.Expect(TOK_PAREN_RIGHT, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected ')'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  // "->"
  // TODO: We should really peek here instead of expecting since ParseBlock will be expecting an
  // open left brace
  res = mLexer.Expect(TOK_DASH, tok);
  if (!res && tok.kind == TOK_BRACE_LEFT) {
    // We dont have a return type so we're done
    return std::make_unique<ProcedurePrototype>(
      std::move(proc_name), "", std::move(args));
  } else if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected \"->\"\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  res = mLexer.Expect(TOK_OP_GT, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected \"->\"\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }

  // IDENTIFIER (return type)
  res = mLexer.Expect(TOK_IDENTIFIER, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected identifier\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }

  std::string return_type = std::get<std::string>(tok.value);
  std::cout << "DEBUG: Lexer consumed identifier: " << return_type << '\n';
  print_tok(tok);

  if (proc_name.empty() || return_type.empty()) {
    return nullptr;
  }

  return std::make_unique<ProcedurePrototype>(
    std::move(proc_name), std::move(return_type), std::move(args));
}

/*
 * ProcedureDeclaration ::= ProcedurePrototype Block
 */
std::unique_ptr<ProcedureDefinition> Parser::ParseProcedureDefintion(std::string proc_name) {
  auto proto = ParseProcedurePrototype(std::move(proc_name));
  if (!proto)
    return nullptr;

  // Block
  auto block = ParseBlock();
  if (!block)
    return nullptr;

  return std::make_unique<ProcedureDefinition>(std::move(proto),
                                               std::move(block));
}

/*
* StructDefinition ::= IDENTIFIER "::" "struct" "{" StructMemberList "}"
*/
std::unique_ptr<StructDefinition> Parser::ParseStructDefinition(std::string struct_name) {
  Token tok;

  // '{'
  bool res = mLexer.Expect(TOK_BRACE_LEFT, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected '{'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  std::vector<std::string> members;
  // ParseStructMembers();

  tok = mLexer.GetNextToken();
  if (tok.kind != TOK_BRACE_RIGHT) {
    warn("[Parse Error] %s:<%d:%d>: Expected '}'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  return std::make_unique<StructDefinition>(std::move(struct_name), std::move(members));
}

/*
* Block ::= "{" { Statement } "}"
*/
std::unique_ptr<Block> Parser::ParseBlock() {
  // '{'
  Token tok;
  bool res = mLexer.Expect(TOK_BRACE_LEFT, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected '{'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  // TODO: Blocks need to parse multiple statements
  std::vector<std::unique_ptr<Statement>> stmts;
  for (auto stmt = ParseStatement(); stmt != nullptr; stmt = ParseStatement()) {
    stmts.push_back(std::move(stmt));
  }

  // '}'
  tok = mLexer.GetToken();
  if (tok.kind != TOK_BRACE_RIGHT) {
    warn("[Parse Error] %s:<%d:%d>: Expected '}'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  return std::make_unique<Block>(std::move(stmts));
}

/*
 * Statement ::= BasicStatement ";"
 */
std::unique_ptr<Statement> Parser::ParseStatement() {
  // BasicStatement
  std::unique_ptr<Statement> stmt = ParseBasicStatement();
  if (!stmt)
    return nullptr;

  // ';'
  Token tok;
  bool res = mLexer.Expect(TOK_SEMICOLON, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected ';'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);
  return stmt;
}

/*
 * BasicStatement ::= ReturnStatement
 */
std::unique_ptr<Statement> Parser::ParseBasicStatement() {
  Token tok = mLexer.GetNextToken();
  if (tok.kind == TOK_ERROR) {
    return nullptr;
  }

  switch (tok.kind) {
  case TOK_KEYWORD_RETURN: {
    print_tok(tok);
    auto return_stmt = ParseReturnStatement();
    if (!return_stmt)
      return nullptr;
    return return_stmt;
  }

  default: return nullptr;
  }
}

/*
 * ReturnStatement ::= "return" Expression
 */
std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
  std::unique_ptr<Expression> expr = ParseExpression();
  if (!expr)
    return nullptr;
  return std::make_unique<ReturnStatement>(std::move(expr));
}

/*
* Expression ::= IntegerLiteral | FloatLiteral | StringLiteral
*/
std::unique_ptr<Expression> Parser::ParseExpression() {
  Token tok = mLexer.GetNextToken();
  if (tok.kind == TOK_ERROR) {
    return nullptr;
  }

  switch (tok.kind) {
  case TOK_INT_LITERAL: {
    auto i = std::get<int>(tok.value);
    std::cout << "DEBUG: Lexer consumed int: " << i << '\n';
    print_tok(tok);
    return std::make_unique<IntegerLiteral>(i);
  }

  case TOK_FLOAT_LITERAL: {
    auto f = std::get<float>(tok.value);
    std::cout << "DEBUG: Lexer consumed float: " << f << '\n';
    print_tok(tok);
    return std::make_unique<FloatLiteral>(f);
  }

  case TOK_STRING: {
    auto s = std::get<std::string>(tok.value);
    std::cout << "DEBUG: Lexer consumed string: \"" << s << "\"\n";
    print_tok(tok);
    return std::make_unique<StringLiteral>(std::move(s));
  }

  default: {
    return nullptr;
  }
  }
}

}  // namespace charlie
