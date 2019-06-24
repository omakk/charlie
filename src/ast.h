#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

namespace charlie {

// Forward declare
class Ast;
class Module;
class Block;
class FunctionDef;
class Expression;
class IntegerLiteral;
class FloatLiteral;
class Statement;
class ReturnStatement;

//===----------------------------------------------------------------------===//
// Visitors
//===----------------------------------------------------------------------===//

class AstVisitor {
public:
  virtual void Visit(Module &ast) = 0;
  virtual void Visit(Block &block) = 0;
  virtual void Visit(FunctionDef &func_def) = 0;
  virtual void Visit(IntegerLiteral &intlit) = 0;
  virtual void Visit(FloatLiteral &floatlit) = 0;
  virtual void Visit(ReturnStatement &retstmt) = 0;

  virtual ~AstVisitor() = default;
};

class AstDisplayVisitor : public AstVisitor {
  std::ostream &mDisplay;
  uint16_t mIndent;

public:
  static constexpr uint16_t kDefaultIndentSpaces = 2;

  AstDisplayVisitor(std::ostream &display = std::cout, uint16_t indent = 0);

  virtual void Visit(Module &mod) override;
  virtual void Visit(Block &block) override;
  virtual void Visit(FunctionDef &func_def) override;
  virtual void Visit(IntegerLiteral &intlit) override;
  virtual void Visit(FloatLiteral &floatlit) override;
  virtual void Visit(ReturnStatement &retstmt) override;
};

//===----------------------------------------------------------------------===//
// AST data structures
//===----------------------------------------------------------------------===//

class Ast {
public:
  virtual void Accept(AstVisitor &v) = 0;
  virtual ~Ast() = default;
};

class Module : public Ast {
public:
  const std::string mName;
  std::unique_ptr<FunctionDef> mFunctionDef;

  Module(const std::string name, std::unique_ptr<FunctionDef> func_def);

  virtual void Accept(AstVisitor &v) override;
};

class FunctionDef : public Ast {
public:
  const std::string mName;
  const std::string mReturnType;
  std::vector<std::string> mArguments;
  std::unique_ptr<Block> mBlock;

  FunctionDef(std::string name,
              std::string return_type,
              std::vector<std::string> args,
              std::unique_ptr<Block> block);

  virtual void Accept(AstVisitor &v) override;
};

class Block : public Ast {
public:
  std::vector<std::unique_ptr<Statement>> mStatements;

  Block(std::vector<std::unique_ptr<Statement>> stmts);

  virtual void Accept(AstVisitor &v) override;
};

//===----------------------------------------------------------------------===//
// Expressions
//===----------------------------------------------------------------------===//

enum ExpressionKind {
  INT_LITERAL,
  FLOAT_LITERAL,
};

class Expression {
public:
  ExpressionKind mExprKind;
  Expression(ExpressionKind kind);
  virtual ~Expression() = default;
};

class IntegerLiteral : public Expression, public Ast {
public:
  int mInt;

  IntegerLiteral(int value, ExpressionKind kind = INT_LITERAL);

  virtual void Accept(AstVisitor &v) override;
};

class FloatLiteral : public Expression, public Ast {
public:
  float mFloat;

  FloatLiteral(float value, ExpressionKind kind = FLOAT_LITERAL);

  virtual void Accept(AstVisitor &v) override;
};

//===----------------------------------------------------------------------===//
// Statements
//===----------------------------------------------------------------------===//

enum StatementKind {
  RETURN,
};

class Statement {
public:
  StatementKind mStmtKind;
  Statement(StatementKind kind);
  virtual ~Statement() = default;
};

class ReturnStatement : public Statement, public Ast {
public:
  std::unique_ptr<Expression> mReturnExpr;

  ReturnStatement(std::unique_ptr<Expression> expr,
                  StatementKind kind = RETURN);

  virtual void Accept(AstVisitor &v) override;
};

}  // namespace charlie
