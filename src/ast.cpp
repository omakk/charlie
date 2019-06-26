#include "ast.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Verifier.h>

#include <sstream>

namespace charlie {

AstDisplayVisitor::AstDisplayVisitor(std::ostream &display, uint16_t indent) :
    mDisplay(display), mIndent(indent) {}

void AstDisplayVisitor::Visit(Module &mod) {
  for (const auto &decl : mod.TopLevelDecls()) {
    switch (decl->mDeclKind) {
    case TopLevelDeclaration::FUNCTION_DEF: {
      auto fdef = static_cast<FunctionDefinition *>(decl.get());
      fdef->Accept(*this);
      break;
    }
    default:
      break;
    };
  }
}

void AstDisplayVisitor::Visit(FunctionPrototype &proto) {
  std::stringstream s;
  s << std::string(mIndent, ' ') << "fun " << proto.Name() << "(";
  // TODO: Handle function arguments
  // if (!func_def.mArguments.empty()) {}
  s << ") " << proto.ReturnType() << '\n';
  mDisplay << s.str();
}

void AstDisplayVisitor::Visit(FunctionDefinition &func_def) {
  func_def.Prototype()->Accept(*this);
  func_def.BodyBlock()->Accept(*this);
}

void AstDisplayVisitor::Visit(Block &block) {
  std::string spaces(mIndent, ' ');
  mDisplay << spaces << "{\n";
  mIndent += kDefaultIndentSpaces;
  for (auto &s : block.Statements()) {
    switch (s->mStmtKind) {
    case StatementKind::RETURN: {
      auto return_stmt = static_cast<ReturnStatement *>(s.get());
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
  case ExpressionKind::INT_LITERAL: {
    auto intlit = static_cast<IntegerLiteral *>(retstmt.mReturnExpr.get());
    intlit->Accept(*this);
    break;
  }
  case ExpressionKind::FLOAT_LITERAL: {
    auto floatlit = static_cast<FloatLiteral *>(retstmt.mReturnExpr.get());
    floatlit->Accept(*this);
    break;
  }
  default:
    break;
  };
  mDisplay << ";\n";
}

CodegenVisitor::CodegenVisitor() : mLLVMIrBuilder(mLLVMContext) {}

void CodegenVisitor::Visit(Module &mod) {
  mLLVMModule = std::make_unique<llvm::Module>(mod.Name(), mLLVMContext);
  for (const auto &decl : mod.TopLevelDecls()) {
    switch (decl->mDeclKind) {
    case TopLevelDeclaration::FUNCTION_DEF: {
      auto fdef = static_cast<FunctionDefinition *>(decl.get());
      fdef->Accept(*this);
      break;
    }
    default:
      break;
    };
  }
  mLLVMModule->print(llvm::errs(), nullptr);
}

void CodegenVisitor::Visit(FunctionPrototype &proto) {
  llvm::Function *f = mLLVMModule->getFunction(proto.Name());
  if (!f) {
    // TODO(oakkila): Assuming all return types are int32
    llvm::Type *return_type = llvm::Type::getInt32Ty(mLLVMContext);
    // TODO(oakkila): Assuming we dont have arguments
    llvm::FunctionType *ft =
      llvm::FunctionType::get(return_type, std::vector<llvm::Type *>(), false);
    f = llvm::Function::Create(
      ft, llvm::Function::ExternalLinkage, proto.Name(), mLLVMModule.get());

    unsigned i = 0;
    for (auto &arg : f->args()) {
      arg.setName(proto.Args().at(i));
      i++;
    }
  }
  mLLVMFunction = f;
}

void CodegenVisitor::Visit(FunctionDefinition &func_def) {
  func_def.Prototype()->Accept(*this);

  llvm::Function *f = mLLVMFunction;
  if (!f) {
    mLLVMFunction = nullptr;
    return;
  }

  llvm::BasicBlock *bb = llvm::BasicBlock::Create(mLLVMContext, "entry", f);
  mLLVMIrBuilder.SetInsertPoint(bb);

  Block *body = func_def.BodyBlock().get();
  if (body) {
    body->Accept(*this);

    llvm::Value *return_value = mLLVMValue;
    mLLVMIrBuilder.CreateRet(return_value);

    llvm::verifyFunction(*f);

    mLLVMFunction = f;
    return;
  }

  f->eraseFromParent();
  mLLVMFunction = nullptr;
}

void CodegenVisitor::Visit(Block &block) {
  for (auto &s : block.Statements()) {
    switch (s->mStmtKind) {
    case StatementKind::RETURN: {
      auto return_stmt = static_cast<ReturnStatement *>(s.get());
      return_stmt->Accept(*this);
      break;
    }
    default:
      break;
    };
  }
}

void CodegenVisitor::Visit(IntegerLiteral &intlit) {
  mLLVMValue = llvm::ConstantInt::get(
    mLLVMContext, llvm::APInt(32 /*numBits*/, intlit.mInt, true /*isSigned*/));
}

void CodegenVisitor::Visit(FloatLiteral &floatlit) {
  mLLVMValue =
    llvm::ConstantFP::get(mLLVMContext, llvm::APFloat(floatlit.mFloat));
}

void CodegenVisitor::Visit(ReturnStatement &retstmt) {
  switch (retstmt.mReturnExpr->mExprKind) {
  case ExpressionKind::INT_LITERAL: {
    auto intlit = static_cast<IntegerLiteral *>(retstmt.mReturnExpr.get());
    intlit->Accept(*this);
    break;
  }
  case ExpressionKind::FLOAT_LITERAL: {
    auto floatlit = static_cast<FloatLiteral *>(retstmt.mReturnExpr.get());
    floatlit->Accept(*this);
    break;
  }
  default:
    break;
  };
}

Module::Module(
  const std::string name,
  std::vector<std::unique_ptr<TopLevelDeclaration>> top_level_decls) :
    mName(std::move(name)),
    mTopLevelDecls(std::move(top_level_decls)) {}

void Module::Accept(AstVisitor &v) {
  v.Visit(*this);
}

FunctionPrototype::FunctionPrototype(std::string name,
                                     std::string return_type,
                                     std::vector<std::string> args) :
    mName(std::move(name)),
    mReturnType(std::move(return_type)),
    mArguments(std::move(args)) {}

void FunctionPrototype::Accept(AstVisitor &v) {
  v.Visit(*this);
}

TopLevelDeclaration::TopLevelDeclaration(DeclKind kind) : mDeclKind(kind) {}

FunctionDefinition::FunctionDefinition(std::unique_ptr<FunctionPrototype> proto,
                                       std::unique_ptr<Block> block,
                                       DeclKind kind) :
    TopLevelDeclaration(kind),
    mProto(std::move(proto)), mBlock(std::move(block)) {}

void FunctionDefinition::Accept(AstVisitor &v) {
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
