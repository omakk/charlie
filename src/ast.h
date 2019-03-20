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
  static constexpr uint16_t kDefaultIndentSpaces = 2;

public:
  AstDisplayVisitor(std::ostream &display = std::cout, uint16_t indent = 0);

  virtual void Visit(Module &mod) override;
  virtual void Visit(Block &block) override;
  virtual void Visit(FunctionDef &func_def) override;
  virtual void Visit(IntegerLiteral &intlit) override;
  virtual void Visit(FloatLiteral &floatlit) override;
  virtual void Visit(ReturnStatement &retstmt) override;
};

class CodegenVisitor : public AstVisitor {
  // LLVM objects
  llvm::LLVMContext mLLVMContext;
  llvm::IRBuilder<> mLLVMIrBuilder;
  std::unique_ptr<llvm::Module> mLLVMModule;
  llvm::Value *mLLVMValue;        // Set after any AST node codegen
  llvm::Function *mLLVMFunction;  // Set after FunctionDef codegen

public:
  CodegenVisitor();

  void Visit(Module &mod) override;
  void Visit(Block &block) override;
  void Visit(FunctionDef &func_def) override;
  void Visit(IntegerLiteral &intlit) override;
  void Visit(FloatLiteral &floatlit) override;
  void Visit(ReturnStatement &retstmt) override;
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
  Module(const std::string name, std::unique_ptr<FunctionDef> func_def);

  const std::string &Name() const {
    return mName;
  }
  std::unique_ptr<FunctionDef> &FuncDef() {
    return mFunctionDef;
  }

  virtual void Accept(AstVisitor &v) override;

private:
  const std::string mName;
  std::unique_ptr<FunctionDef> mFunctionDef;
};

class FunctionDef : public Ast {
public:
  FunctionDef(std::string name,
              std::string return_type,
              std::vector<std::string> args,
              std::unique_ptr<Block> block);

  const std::string &Name() const {
    return mName;
  }
  const std::string &ReturnType() const {
    return mReturnType;
  }
  const std::vector<std::string> &Args() const {
    return mArguments;
  }
  std::unique_ptr<Block> &BodyBlock() {
    return mBlock;
  }

  virtual void Accept(AstVisitor &v) override;

private:
  const std::string mName;
  const std::string mReturnType;
  const std::vector<std::string> mArguments;
  std::unique_ptr<Block> mBlock;
};

class Block : public Ast {
public:
  Block(std::vector<std::unique_ptr<Statement>> stmts);

  const std::vector<std::unique_ptr<Statement>> &Statements() const {
    return mStatements;
  }

  virtual void Accept(AstVisitor &v) override;

private:
  const std::vector<std::unique_ptr<Statement>> mStatements;
};

//===----------------------------------------------------------------------===//
// Expressions
//===----------------------------------------------------------------------===//

enum class ExpressionKind {
  INT_LITERAL,
  FLOAT_LITERAL,
};

class Expression {
public:
  ExpressionKind mExprKind;
  virtual ~Expression() = default;

protected:
  Expression(ExpressionKind kind);
};

class IntegerLiteral : public Expression, public Ast {
public:
  int mInt;

  IntegerLiteral(int value, ExpressionKind kind = ExpressionKind::INT_LITERAL);

  virtual void Accept(AstVisitor &v) override;
};

class FloatLiteral : public Expression, public Ast {
public:
  float mFloat;

  FloatLiteral(float value,
               ExpressionKind kind = ExpressionKind::FLOAT_LITERAL);

  virtual void Accept(AstVisitor &v) override;
};

//===----------------------------------------------------------------------===//
// Statements
//===----------------------------------------------------------------------===//

enum class StatementKind {
  RETURN,
};

class Statement {
public:
  StatementKind mStmtKind;
  virtual ~Statement() = default;

protected:
  Statement(StatementKind kind);
};

class ReturnStatement : public Statement, public Ast {
public:
  std::unique_ptr<Expression> mReturnExpr;

  ReturnStatement(std::unique_ptr<Expression> expr,
                  StatementKind kind = StatementKind::RETURN);

  virtual void Accept(AstVisitor &v) override;
};

}  // namespace charlie
