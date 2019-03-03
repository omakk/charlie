#include "ast.h"

namespace charlie {

Module::Module(const std::string &name, std::unique_ptr<FunctionDef> func_def)
    : mName(name), mFunctionDef(std::move(func_def)) {}

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
