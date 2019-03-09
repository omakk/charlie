#include "ast.h"
#include <sstream>

namespace charlie {

AstDisplayVisitor::AstDisplayVisitor(std::ostream &display, uint16_t indent)
    : mDisplay(display), mIndent(indent) {}

void AstDisplayVisitor::visit(Module &mod) { mod.mFunctionDef->accept(*this); }

void AstDisplayVisitor::visit(FunctionDef &func_def) {
  std::stringstream s;

  s << std::string(mIndent, ' ') << "fun " << func_def.mName << "(";
  // TODO: Handle function arguments
  //if (!func_def.mArguments.empty()) {}
  s << ") " << func_def.mReturnType << '\n';

  mDisplay << s.str();

  func_def.mBlock->accept(*this);
}

void AstDisplayVisitor::visit(Block &block) {
  std::string spaces(mIndent, ' ');
  mDisplay << spaces << "{\n";
  mIndent += kDefaultIndentSpaces;
  for (auto &s : block.mStatements) {
    switch (s->mStmtKind) {
    case RETURN: {
      auto return_stmt = dynamic_cast<ReturnStatement *>(s.get());
      return_stmt->accept(*this);
      break;
    }
    default:
      break;
    };
  }
  mIndent -= kDefaultIndentSpaces;
  mDisplay << spaces << "}\n";
}

void AstDisplayVisitor::visit(IntegerLiteral &intlit) {
  mDisplay << intlit.mInt;
}

void AstDisplayVisitor::visit(FloatLiteral &floatlit) {
  mDisplay << floatlit.mFloat;
}

void AstDisplayVisitor::visit(ReturnStatement &retstmt) {
  mDisplay << std::string(mIndent, ' ') << "return ";
  switch (retstmt.mReturnExpr->mExprKind) {
  case INT_LITERAL: {
    auto intlit = dynamic_cast<IntegerLiteral *>(retstmt.mReturnExpr.get());
    intlit->accept(*this);
    break;
  }
  case FLOAT_LITERAL: {
    auto floatlit = dynamic_cast<FloatLiteral *>(retstmt.mReturnExpr.get());
    floatlit->accept(*this);
    break;
  }
  default:
    break;
  };
  mDisplay << ";\n";
}

Module::Module(const std::string name, std::unique_ptr<FunctionDef> func_def)
    : mName(std::move(name)), mFunctionDef(std::move(func_def)) {}

void Module::accept(AstVisitor &v) { v.visit(*this); }

FunctionDef::FunctionDef(std::string name, std::string return_type,
                         std::vector<std::string> args,
                         std::unique_ptr<Block> block)
    : mName(std::move(name)), mReturnType(std::move(return_type)),
      mArguments(std::move(args)), mBlock(std::move(block)) {}

void FunctionDef::accept(AstVisitor &v) { v.visit(*this); }

Block::Block(std::vector<std::unique_ptr<Statement>> stmts)
    : mStatements(std::move(stmts)) {}

void Block::accept(AstVisitor &v) { v.visit(*this); }

Expression::Expression(ExpressionKind kind) : mExprKind(kind) {}

IntegerLiteral::IntegerLiteral(int value, ExpressionKind kind)
    : Expression(kind), mInt(value) {}

void IntegerLiteral::accept(AstVisitor &v) { v.visit(*this); }

FloatLiteral::FloatLiteral(float value, ExpressionKind kind)
    : Expression(kind), mFloat(value) {}

void FloatLiteral::accept(AstVisitor &v) { v.visit(*this); }

Statement::Statement(StatementKind kind) : mStmtKind(kind) {}

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expr,
                                 StatementKind kind)
    : Statement(kind), mReturnExpr(std::move(expr)) {}

void ReturnStatement::accept(AstVisitor &v) { v.visit(*this); }

} // namespace charlie
