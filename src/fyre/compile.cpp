#include "compile.h"
#include "context.h"
#include "AST.h"
#include "exceptions.h"

#include <llvm/ADT/APInt.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>

#include <stdexcept>
#include <memory>
#include <variant>

namespace Fyre {
  // CompiledVal::CompiledVal(ValueT v) : m_value(v) {}


  llvm::Value *IntLit::codegen(Context &ctx) const {
    return llvm::ConstantInt::get(ctx.llvm_ctx(), llvm::APInt(64, m_val, true));
  }

  llvm::Type *Type::codegen(Context &ctx) const {
    if (m_str == "Int")
      return llvm::Type::getInt64Ty(ctx.llvm_ctx());
    else
      throw std::out_of_range("Type not found for id: " + m_str);
  }

  TopLvl::StatementIR FunDec::codegen(Context &ctx) const {
    std::vector<llvm::Type *> arg_types;

    for (auto &arg : m_args)
      arg_types.push_back(arg.type->codegen(ctx));

    llvm::FunctionType *ft =
      llvm::FunctionType::get(m_type->codegen(ctx), arg_types, false);

    llvm::Function *fn =
      llvm::Function::Create(ft,
                             llvm::Function::ExternalLinkage,
                             m_name->str(),
                             &ctx.module());

    int i = 0;
    for (auto &arg : fn->args()) {
      if (m_args[i].name)
        arg.setName((*m_args[i].name)->str());
      i++;
    }

    return fn;
  }

  TopLvl::StatementIR FunDef::codegen(Context &ctx) const {
    llvm::Function *fn;

    if (ctx.has_function(m_name->str())) {
      fn = ctx.function(m_name->str());
    } else {
      std::vector<llvm::Type *> arg_types;

      for (auto &arg : m_args) {
        if (!arg.type)
          throw Compiler::Error("Missing type for argument " + arg.name->str());

        arg_types.push_back((*arg.type)->codegen(ctx));
      }

      llvm::FunctionType *ft =
        llvm::FunctionType::get(m_type->codegen(ctx), arg_types, false);

      fn = llvm::Function::Create(ft,
                                  llvm::Function::ExternalLinkage,
                                  m_name->str(),
                                  &ctx.module());
    }

    ContextChild scope = ctx.make_frame();
    int i = 0;
    for (auto &arg : fn->args()) {
      scope.value(m_args[i].name->str(), &arg);
      arg.setName(m_args[i++].name->str());
    }

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx.llvm_ctx(), "entry", fn);
    ctx.builder().SetInsertPoint(entry);

    ctx.builder().CreateRet(m_expr->codegen(scope));

    // TODO: use the return val of this
    llvm::verifyFunction(*fn);

    return fn;
  }

  llvm::Value *FunCal::codegen(Context &ctx) const {
    llvm::Function *fn = ctx.function(m_name->str());

    std::vector<llvm::Value *> args;
    for (auto &arg : m_args)
      args.push_back(arg->codegen(ctx));

    return ctx.builder().CreateCall(fn, args, "calltmp");
  }

  llvm::Value *Var::codegen(Context &ctx) const {
    return ctx.value(m_name->str());
  }

  std::unique_ptr<ContextRoot> Module::codegen(const std::string &module_name) const {
    auto ctx = std::make_unique<ContextRoot>(module_name);

    for (auto toplvl : m_statements) {
      auto ir =  toplvl->codegen(*ctx);

      // Handle Function*
      using F = llvm::Function *;
      if (std::holds_alternative<F>(ir)) {
        F fn = std::get<F>(ir);

        ctx->function(fn->getName(), fn);
      }
    }

    // return module;
    return ctx;
  }
}
