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
            << tok.span.pos_end << ")\n";
}

Parser::Parser(std::string file) :
    mFileName(std::move(file)), mLexer(mFileName) {}

Parser::~Parser() {}

std::unique_ptr<Module> Parser::Parse() {
  std::vector<std::unique_ptr<TopLevelDeclaration>> decls;
  auto decl = ParseTopLevelDeclaration();
  if (!decl)
    return nullptr;
  decls.push_back(std::move(decl));
  return std::make_unique<Module>(std::string(mFileName), std::move(decls));
}

/*
 * TopLevelDeclaration ::= FunctionDefinition
 */
std::unique_ptr<TopLevelDeclaration> Parser::ParseTopLevelDeclaration() {
  // TODO(oakkila): Only dealing with FunctionDefinition here
  auto func_def = ParseFunctionDefintion();
  if (!func_def)
    return nullptr;
  return func_def;
}

/*
 * FunctionPrototype ::=
 *      "fn" IDENTIFIER '(' FunctionParameters* ')' IDENTIFIER
 */
std::unique_ptr<FunctionPrototype> Parser::ParseFunctionPrototype() {
  // "fun"
  Token tok;
  bool res = mLexer.Expect(TOK_KEYWORD_FUN, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected keyword 'fun'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  // IDENTIFIER (function name)
  res = mLexer.Expect(TOK_IDENTIFIER, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected identifier\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  std::string fn_name;
  auto pvalue = std::get_if<std::string>(&tok.value);
  if (pvalue)
    fn_name = *pvalue;
  std::cout << "DEBUG: Lexer consumed identifier: " << fn_name << '\n';
  print_tok(tok);

  // '('
  res = mLexer.Expect(TOK_PAREN_LEFT, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected '('\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);

  std::vector<std::string> args;
  // ParseFunctionParameters();

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

  // IDENTIFIER (return type)
  res = mLexer.Expect(TOK_IDENTIFIER, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected identifier\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }

  std::string return_type;
  pvalue = std::get_if<std::string>(&tok.value);
  if (pvalue)
    return_type = *pvalue;
  std::cout << "DEBUG: Lexer consumed identifier: " << return_type << '\n';
  print_tok(tok);

  if (fn_name.empty() || return_type.empty()) {
    return nullptr;
  }

  return std::make_unique<FunctionPrototype>(std::move(fn_name),
                                             std::move(return_type),
                                             std::move(args));
}

/*
 * FunctionDeclaration ::= FunctionPrototype Block
 */
std::unique_ptr<FunctionDefinition> Parser::ParseFunctionDefintion() {
  auto proto = ParseFunctionPrototype();
  if (!proto)
    return nullptr;

  // Block
  auto block = ParseBlock();
  if (!block)
    return nullptr;

  return std::make_unique<FunctionDefinition>(std::move(proto),
                                              std::move(block));
}

/*
 * Block ::= '{' Statement* '}'
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
  auto stmt = ParseStatement();
  if (stmt)
    stmts.push_back(std::move(stmt));

  // '}'
  res = mLexer.Expect(TOK_BRACE_RIGHT, tok);
  if (!res) {
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
 * Statement ::= BasicStatement ';'
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
  return std::move(stmt);
}

/*
 * BasicStatement ::=
 *      ReturnStatement
 */
std::unique_ptr<Statement> Parser::ParseBasicStatement() {
  std::unique_ptr<Statement> return_stmt = ParseReturnStatement();
  if (!return_stmt)
    return nullptr;
  return std::move(return_stmt);
}

/*
 * ReturnStatement ::= "return" Expression
 */
std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
  Token tok;
  bool res = mLexer.Expect(TOK_KEYWORD_RETURN, tok);
  if (!res) {
    warn("[Parse Error] %s:<%d:%d>: Expected keyword 'return'\n",
         mFileName.c_str(),
         tok.span.line_start,
         tok.span.pos_start);
    return nullptr;
  }
  print_tok(tok);
  std::unique_ptr<Expression> expr = ParseExpression();
  if (!expr)
    return nullptr;
  return std::make_unique<ReturnStatement>(std::move(expr));
}

/*
 * Expression ::=
 *      | IntegerLiteral
 *      | FloatLiteral
 */
std::unique_ptr<Expression> Parser::ParseExpression() {
  Token tok = mLexer.GetNextToken();
  if (tok.kind == TOK_INT_LITERAL) {
    auto pvalue = std::get_if<int>(&tok.value);
    if (!pvalue) {
      std::cout << "DEBUG: Failed to get int variant from Token\n";
      return nullptr;
    }
    int i = *pvalue;
    std::cout << "DEBUG: Lexer consumed int: " << i << '\n';
    print_tok(tok);
    return std::make_unique<IntegerLiteral>(i);
  } else if (tok.kind == TOK_FLOAT_LITERAL) {
    auto pvalue = std::get_if<float>(&tok.value);
    if (!pvalue) {
      std::cout << "DEBUG: Failed to get float variant from Token\n";
      return nullptr;
    }
    float f = *pvalue;
    std::cout << "DEBUG: Lexer consumed float: " << f << '\n';
    print_tok(tok);
    return std::make_unique<FloatLiteral>(f);
  } else {
    std::cout << "DEBUG: Lexer did not consume experssion\n";
    return nullptr;
  }
}

}  // namespace charlie
