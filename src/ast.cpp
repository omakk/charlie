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
    case TopLevelDeclaration::PROC_DEF: {
      auto pdef = static_cast<ProcedureDefinition *>(decl.get());
      pdef->Accept(*this);
      break;
    }
    case TopLevelDeclaration::STRUCT_DEF: {
      auto sdef = static_cast<StructDefinition *>(decl.get());
      sdef->Accept(*this);
      break;
    }
    default: break;
    };
  }
}

void AstDisplayVisitor::Visit(ProcedurePrototype &proto) {
  std::stringstream s;
  s << std::string(mIndent, ' ') <<  proto.Name() << " :: proc(";
  // TODO: Handle function arguments
  // if (!proc_def.mArguments.empty()) {}
  if (proto.ReturnType().empty()) {
    s << ") \n";
  } else {
    s << ") -> " << proto.ReturnType() << '\n';
  }
  mDisplay << s.str();
}

void AstDisplayVisitor::Visit(ProcedureDefinition &proc_def) {
  proc_def.Prototype()->Accept(*this);
  proc_def.BodyBlock()->Accept(*this);
}

void AstDisplayVisitor::Visit(StructDefinition &struct_def) {
  std::stringstream s;
  s << std::string(mIndent, ' ') <<  struct_def.Name() << " :: struct {\n";
  // TODO: Handle non-comma-terminated case
  mIndent += kDefaultIndentSpaces;
  for (auto &member : struct_def.Members()) {
    s << std::string(mIndent, ' ')
      <<  member.name << ": " << member.type << ",\n";
  }
  mIndent -= kDefaultIndentSpaces;
  s << "}\n";
  mDisplay << s.str();
}

void AstDisplayVisitor::Visit(Block &block) {
  std::string spaces(mIndent, ' ');
  mDisplay << spaces << "{\n";
  mIndent += kDefaultIndentSpaces;
  for (auto &s : block.Statements()) {
    switch (s->mStmtKind) {
    case Statement::RETURN: {
      auto return_stmt = static_cast<ReturnStatement *>(s.get());
      return_stmt->Accept(*this);
      break;
    }
    default: break;
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

void AstDisplayVisitor::Visit(StringLiteral &strlit) {
  mDisplay << '"' << strlit.mString << '"';
}

void AstDisplayVisitor::Visit(ReturnStatement &retstmt) {
  mDisplay << std::string(mIndent, ' ') << "return ";
  switch (retstmt.mReturnExpr->mExprKind) {
  case Expression::INT_LITERAL: {
    auto intlit = static_cast<IntegerLiteral *>(retstmt.mReturnExpr.get());
    intlit->Accept(*this);
    break;
  }
  case Expression::FLOAT_LITERAL: {
    auto floatlit = static_cast<FloatLiteral *>(retstmt.mReturnExpr.get());
    floatlit->Accept(*this);
    break;
  }
  case Expression::STRING_LITERAL: {
    auto strlit = static_cast<StringLiteral *>(retstmt.mReturnExpr.get());
    strlit->Accept(*this);
    break;
  }
  default: break;
  };
  mDisplay << ";\n";
}

CodegenVisitor::CodegenVisitor() : mLLVMIrBuilder(mLLVMContext) {}

void CodegenVisitor::Visit(Module &mod) {
  mLLVMModule = std::make_unique<llvm::Module>(mod.Name(), mLLVMContext);
  for (const auto &decl : mod.TopLevelDecls()) {
    switch (decl->mDeclKind) {
    case TopLevelDeclaration::PROC_DEF: {
      auto pdef = static_cast<ProcedureDefinition *>(decl.get());
      pdef->Accept(*this);
      break;
    }
    case TopLevelDeclaration::STRUCT_DEF: {
      auto sdef = static_cast<StructDefinition *>(decl.get());
      sdef->Accept(*this);
      break;
    }
    default: break;
    };
  }
  mLLVMModule->print(llvm::errs(), nullptr);
}

void CodegenVisitor::Visit(ProcedurePrototype &proto) {
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

void CodegenVisitor::Visit(ProcedureDefinition &proc_def) {
  proc_def.Prototype()->Accept(*this);

  llvm::Function *f = mLLVMFunction;
  if (!f) {
    mLLVMFunction = nullptr;
    return;
  }

  llvm::BasicBlock *bb = llvm::BasicBlock::Create(mLLVMContext, "entry", f);
  mLLVMIrBuilder.SetInsertPoint(bb);

  Block *body = proc_def.BodyBlock().get();
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

void CodegenVisitor::Visit(StructDefinition &struct_def) {
  // TODO: struct codegen
  (void) struct_def;
}

void CodegenVisitor::Visit(Block &block) {
  for (auto &s : block.Statements()) {
    switch (s->mStmtKind) {
    case Statement::RETURN: {
      auto return_stmt = static_cast<ReturnStatement *>(s.get());
      return_stmt->Accept(*this);
      break;
    }
    default: break;
    };
  }
}

void CodegenVisitor::Visit(IntegerLiteral &intlit) {
  mLLVMValue = llvm::ConstantInt::get(
    mLLVMContext, llvm::APInt(/*numBits=*/32, intlit.mInt, /*isSigned=*/true));
}

void CodegenVisitor::Visit(FloatLiteral &floatlit) {
  mLLVMValue =
    llvm::ConstantFP::get(mLLVMContext, llvm::APFloat(floatlit.mFloat));
}

void CodegenVisitor::Visit(StringLiteral &strlit) {
  llvm::Type *i8_array_type =
    llvm::ArrayType::get(llvm::IntegerType::get(mLLVMContext, /*numbits=*/8),
                         strlit.mString.length());
  if (!i8_array_type) {
    mLLVMValue = nullptr;
    return;
  }

  llvm::Module *mod = mLLVMModule.get();
  assert(mod);

  // FIXME: global_str will leak
  llvm::GlobalVariable *global_str = new llvm::GlobalVariable(
    /*Module=*/*mod,
    /*Type=*/i8_array_type,
    /*isConstant=*/true,
    /*Linkage=*/llvm::GlobalVariable::PrivateLinkage,
    /*Initializer=*/nullptr,
    /*Name=*/".str");
  if (!global_str) {
    mLLVMValue = nullptr;
    return;
  }

  global_str->setAlignment(1);

  llvm::Constant *const_array = llvm::ConstantDataArray::getString(
    mLLVMContext, strlit.mString.c_str(), /*AddNull=*/false);

  global_str->setInitializer(const_array);

  llvm::Constant *const_int64_0 = llvm::ConstantInt::get(
    mLLVMContext, llvm::APInt(/*numbits=*/64, 0, /*issigned=*/true));

  std::vector<llvm::Constant *> const_ptr_indices(2, const_int64_0);
  llvm::Constant *const_ptr = llvm::ConstantExpr::getGetElementPtr(
    i8_array_type, global_str, const_ptr_indices, /*inBounds=*/true);

  mLLVMValue = const_ptr;
}

void CodegenVisitor::Visit(ReturnStatement &retstmt) {
  switch (retstmt.mReturnExpr->mExprKind) {
  case Expression::INT_LITERAL: {
    auto intlit = static_cast<IntegerLiteral *>(retstmt.mReturnExpr.get());
    intlit->Accept(*this);
    break;
  }
  case Expression::FLOAT_LITERAL: {
    auto floatlit = static_cast<FloatLiteral *>(retstmt.mReturnExpr.get());
    floatlit->Accept(*this);
    break;
  }
  case Expression::STRING_LITERAL: {
    auto strlit = static_cast<StringLiteral *>(retstmt.mReturnExpr.get());
    strlit->Accept(*this);
    break;
  }
  default: break;
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

ProcedurePrototype::ProcedurePrototype(std::string name,
                                       std::string return_type,
                                       std::vector<std::string> args) :
    mName(std::move(name)),
    mReturnType(std::move(return_type)), mArguments(std::move(args)) {}

void ProcedurePrototype::Accept(AstVisitor &v) {
  v.Visit(*this);
}

TopLevelDeclaration::TopLevelDeclaration(DeclKind kind) : mDeclKind(kind) {}

ProcedureDefinition::ProcedureDefinition(std::unique_ptr<ProcedurePrototype> proto,
                                       std::unique_ptr<Block> block,
                                       DeclKind kind) :
    TopLevelDeclaration(kind),
    mProto(std::move(proto)), mBlock(std::move(block)) {}

void ProcedureDefinition::Accept(AstVisitor &v) {
  v.Visit(*this);
}

StructDefinition::StructDefinition(std::string struct_name,
                                   std::vector<StructDefinition::StructMember> members,
                                   DeclKind kind) :
    TopLevelDeclaration(kind),
    mStructName(std::move(struct_name)), mMembers(std::move(members)) {}

void StructDefinition::Accept(AstVisitor &v) {
  v.Visit(*this);
}

Block::Block(std::vector<std::unique_ptr<Statement>> stmts) :
    mStatements(std::move(stmts)) {}

void Block::Accept(AstVisitor &v) {
  v.Visit(*this);
}

Expression::Expression(ExprKind kind) : mExprKind(kind) {}

IntegerLiteral::IntegerLiteral(int value, ExprKind kind) :
    Expression(kind), mInt(value) {}

void IntegerLiteral::Accept(AstVisitor &v) {
  v.Visit(*this);
}

FloatLiteral::FloatLiteral(float value, ExprKind kind) :
    Expression(kind), mFloat(value) {}

void FloatLiteral::Accept(AstVisitor &v) {
  v.Visit(*this);
}

StringLiteral::StringLiteral(std::string value, ExprKind kind) :
    Expression(kind), mString(std::move(value)) {}

void StringLiteral::Accept(AstVisitor &v) {
  v.Visit(*this);
}

Statement::Statement(StmtKind kind) : mStmtKind(kind) {}

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expr,
                                 StmtKind kind) :
    Statement(kind),
    mReturnExpr(std::move(expr)) {}

void ReturnStatement::Accept(AstVisitor &v) {
  v.Visit(*this);
}

}  // namespace charlie
