#include "ast.h"

#include <sstream>

namespace charlie {

AstDisplayVisitor::AstDisplayVisitor(std::ostream &display, uint16_t indent) :
    mDisplay(display), mIndent(indent) {}

void AstDisplayVisitor::Visit(Module &mod) {
  mod.mFunctionDef->Accept(*this);
}

void AstDisplayVisitor::Visit(FunctionDef &func_def) {
  std::stringstream s;

  s << std::string(mIndent, ' ') << "fun " << func_def.mName << "(";
  // TODO: Handle function arguments
  // if (!func_def.mArguments.empty()) {}
  s << ") " << func_def.mReturnType << '\n';

  mDisplay << s.str();

  func_def.mBlock->Accept(*this);
}

void AstDisplayVisitor::Visit(Block &block) {
  std::string spaces(mIndent, ' ');
  mDisplay << spaces << "{\n";
  mIndent += kDefaultIndentSpaces;
  for (auto &s : block.mStatements) {
    switch (s->mStmtKind) {
    case RETURN: {
      auto return_stmt = dynamic_cast<ReturnStatement *>(s.get());
      return_stmt->Accept(*this);
      break;
    }
    default:
      break;
    };
  }
  mIndent -= kDefaultIndentSpaces;
  mDisplay << spaces << "}\n";
}

void AstDisplayVisitor::Visit(IntegerLiteral &intlit) {
  mDisplay << intlit.mInt;
}

void AstDisplayVisitor::Visit(FloatLiteral &floatlit) {
  mDisplay << floatlit.mFloat;
}

void AstDisplayVisitor::Visit(ReturnStatement &retstmt) {
  mDisplay << std::string(mIndent, ' ') << "return ";
  switch (retstmt.mReturnExpr->mExprKind) {
  case INT_LITERAL: {
    auto intlit = dynamic_cast<IntegerLiteral *>(retstmt.mReturnExpr.get());
    intlit->Accept(*this);
    break;
  }
  case FLOAT_LITERAL: {
    auto floatlit = dynamic_cast<FloatLiteral *>(retstmt.mReturnExpr.get());
    floatlit->Accept(*this);
    break;
  }
  default:
    break;
  };
  mDisplay << ";\n";
}

Module::Module(const std::string name, std::unique_ptr<FunctionDef> func_def) :
    mName(std::move(name)), mFunctionDef(std::move(func_def)) {}

void Module::Accept(AstVisitor &v) {
  v.Visit(*this);
}

FunctionDef::FunctionDef(std::string name,
                         std::string return_type,
                         std::vector<std::string> args,
                         std::unique_ptr<Block> block) :
    mName(std::move(name)),
    mReturnType(std::move(return_type)), mArguments(std::move(args)),
    mBlock(std::move(block)) {}

void FunctionDef::Accept(AstVisitor &v) {
  v.Visit(*this);
}

Block::Block(std::vector<std::unique_ptr<Statement>> stmts) :
    mStatements(std::move(stmts)) {}

void Block::Accept(AstVisitor &v) {
  v.Visit(*this);
}

Expression::Expression(ExpressionKind kind) : mExprKind(kind) {}

IntegerLiteral::IntegerLiteral(int value, ExpressionKind kind) :
    Expression(kind), mInt(value) {}

void IntegerLiteral::Accept(AstVisitor &v) {
  v.Visit(*this);
}

FloatLiteral::FloatLiteral(float value, ExpressionKind kind) :
    Expression(kind), mFloat(value) {}

void FloatLiteral::Accept(AstVisitor &v) {
  v.Visit(*this);
}

Statement::Statement(StatementKind kind) : mStmtKind(kind) {}

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expr,
                                 StatementKind kind) :
    Statement(kind),
    mReturnExpr(std::move(expr)) {}

void ReturnStatement::Accept(AstVisitor &v) {
  v.Visit(*this);
}

}  // namespace charlie
