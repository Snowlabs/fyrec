#ifndef COMPILE_H
#define COMPILE_H

// #include <map>
// #include <optional>

// #include <llvm/IR/IRBuilder.h>
// #include <llvm/IR/BasicBlock.h>
// #include <llvm/IR/Value.h>
// #include <llvm/IR/Function.h>
// #include <llvm/IR/Type.h>

// #include "AST.h"
// #include "context.h"

namespace Fyre {
  // using LLVMContextPtr = std::shared_ptr<llvm::LLVMContext>;


  // class CompiledVal {
  // public:
  //   using ValueT = std::variant<llvm::Value *, llvm::Function *>;

  //   CompiledVal(ValueT);

  //   template<class T>
  //   T get();

  // private:
  //   ValueT m_value;
  // };

  // template<>
  // llvm::Value *CompiledVal::get() {
  //   return std::get<llvm::Value *>(m_value);
  // }

  // template<>
  // llvm::Function *CompiledVal::get() {
  //   return std::get<llvm::Function *>(m_value);
  // }

}

#endif
