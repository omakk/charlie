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
class TopLevelDeclaration;
class Block;
class FunctionPrototype;
class FunctionDefinition;
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
  virtual void Visit(FunctionPrototype &proto) = 0;
  virtual void Visit(FunctionDefinition &func_def) = 0;
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

  void Visit(Module &mod) override;
  void Visit(Block &block) override;
  void Visit(FunctionPrototype &proto) override;
  void Visit(FunctionDefinition &func_def) override;
  void Visit(IntegerLiteral &intlit) override;
  void Visit(FloatLiteral &floatlit) override;
  void Visit(ReturnStatement &retstmt) override;
};

class CodegenVisitor : public AstVisitor {
  // LLVM objects
  llvm::LLVMContext mLLVMContext;
  llvm::IRBuilder<> mLLVMIrBuilder;
  std::unique_ptr<llvm::Module> mLLVMModule;
  llvm::Value *mLLVMValue;        // Set after any AST node codegen
  llvm::Function *mLLVMFunction;  // Set after FunctionDefinition codegen

public:
  CodegenVisitor();

  void Visit(Module &mod) override;
  void Visit(Block &block) override;
  void Visit(FunctionPrototype &proto) override;
  void Visit(FunctionDefinition &func_def) override;
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
  Module(const std::string name,
         std::vector<std::unique_ptr<TopLevelDeclaration>> top_level_decls);

  const std::string &Name() const {
    return mName;
  }
  const std::vector<std::unique_ptr<TopLevelDeclaration>> &
  TopLevelDecls() const {
    return mTopLevelDecls;
  }

  virtual void Accept(AstVisitor &v) override;

private:
  const std::string mName;
  const std::vector<std::unique_ptr<TopLevelDeclaration>> mTopLevelDecls;
};

class FunctionPrototype : public Ast {
public:
  FunctionPrototype(std::string name,
                    std::string return_type,
                    std::vector<std::string> args);

  const std::string &Name() const {
    return mName;
  }
  const std::string &ReturnType() const {
    return mReturnType;
  }
  const std::vector<std::string> &Args() const {
    return mArguments;
  }

  virtual void Accept(AstVisitor &v) override;

private:
  const std::string mName;
  const std::string mReturnType;
  const std::vector<std::string> mArguments;
};

//===----------------------------------------------------------------------===//
// Declarations
//===----------------------------------------------------------------------===//

class TopLevelDeclaration {
public:
  enum DeclKind {
    FUNCTION_DEF,
  } mDeclKind;

  virtual ~TopLevelDeclaration() = default;

protected:
  TopLevelDeclaration(DeclKind kind);
};

class FunctionDefinition : public TopLevelDeclaration, public Ast {
public:
  FunctionDefinition(std::unique_ptr<FunctionPrototype> proto,
                     std::unique_ptr<Block> block,
                     DeclKind kind = FUNCTION_DEF);

  std::unique_ptr<FunctionPrototype> &Prototype() {
    return mProto;
  }
  std::unique_ptr<Block> &BodyBlock() {
    return mBlock;
  }

  virtual void Accept(AstVisitor &v) override;

private:
  std::unique_ptr<FunctionPrototype> mProto;
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

class Expression {
public:
  enum ExprKind {
    INT_LITERAL,
    FLOAT_LITERAL,
  } mExprKind;

  virtual ~Expression() = default;

protected:
  Expression(ExprKind kind);
};

class IntegerLiteral : public Expression, public Ast {
public:
  int mInt;

  IntegerLiteral(int value, ExprKind kind = INT_LITERAL);

  virtual void Accept(AstVisitor &v) override;
};

class FloatLiteral : public Expression, public Ast {
public:
  float mFloat;

  FloatLiteral(float value, ExprKind kind = FLOAT_LITERAL);

  virtual void Accept(AstVisitor &v) override;
};

//===----------------------------------------------------------------------===//
// Statements
//===----------------------------------------------------------------------===//

class Statement {
public:
  enum StmtKind {
    RETURN,
  } mStmtKind;

  virtual ~Statement() = default;

protected:
  Statement(StmtKind kind);
};

class ReturnStatement : public Statement, public Ast {
public:
  std::unique_ptr<Expression> mReturnExpr;

  ReturnStatement(std::unique_ptr<Expression> expr, StmtKind kind = RETURN);

  virtual void Accept(AstVisitor &v) override;
};

}  // namespace charlie
