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
  virtual void visit(Module &ast) = 0;
  virtual void visit(Block &ast) = 0;
  virtual void visit(FunctionDef &ast) = 0;
  virtual void visit(IntegerLiteral &ast) = 0;
  virtual void visit(FloatLiteral &ast) = 0;
  virtual void visit(ReturnStatement &ast) = 0;

  virtual ~AstVisitor();
};

//===----------------------------------------------------------------------===//
// AST data structures
//===----------------------------------------------------------------------===//


class Ast {
public:
  virtual void accept(AstVisitor &v) = 0;
  virtual ~Ast() = default;
};

class Module : public Ast {
  std::string mName;
  std::unique_ptr<FunctionDef> mFunctionDef;

public:
  Module(const std::string &name, std::unique_ptr<FunctionDef> func_def);

  virtual void accept(AstVisitor &v) override;
};

class Block : public Ast {
  std::vector<std::unique_ptr<Statement>> mStatements;

public:
  Block(std::vector<std::unique_ptr<Statement>> stmts);

  virtual void accept(AstVisitor &v) override;
};

class FunctionDef : public Ast {
  std::string mName;
  std::string mReturnType;
  std::vector<std::string> mArguments;
  std::unique_ptr<Block> mBlock;

public:
  FunctionDef(std::string name, std::string return_type,
              std::vector<std::string> args, std::unique_ptr<Block> block);

  virtual void accept(AstVisitor &v) override;
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
  int mInt;

public:
  IntegerLiteral(int value, ExpressionKind kind = INT_LITERAL);

  virtual void accept(AstVisitor &v) override;
};

class FloatLiteral : public Expression, public Ast {
  float mFloat;

public:
  FloatLiteral(float value, ExpressionKind kind = FLOAT_LITERAL);

  virtual void accept(AstVisitor &v) override;
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
  std::unique_ptr<Expression> mReturnExpr;

public:
  ReturnStatement(std::unique_ptr<Expression> expr,
                  StatementKind kind = RETURN);

  virtual void accept(AstVisitor &v) override;
};

} // namespace charlie
