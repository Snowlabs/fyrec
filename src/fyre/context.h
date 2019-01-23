#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>
#include <optional>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

namespace Fyre {
  class ContextChild;

  /// A compilation context

  /// This is passed down the codegen process and holds
  /// the llvm builder, context and module. It also
  /// holds the scope and symbol table.
  class Context {
  public:
    virtual ~Context() {};

    /// Get the llvm::LLVMContext
    virtual llvm::LLVMContext &llvm_ctx() = 0;
    /// Get the llvm::IRBuilder
    virtual llvm::IRBuilder<> &builder() = 0;
    /// Get the llvm::Module
    virtual llvm::Module      &module() = 0;

    /// Return whether value exists
    virtual bool has_value   (const std::string &) = 0;
    /// Return whether function exists
    virtual bool has_function(const std::string &) = 0;
    /// Return whether type exists
    virtual bool has_type    (const std::string &) = 0;

    /// Get value by id
    virtual llvm::Value    *value   (const std::string &) = 0;
    /// Get function by id
    virtual llvm::Function *function(const std::string &) = 0;
    /// Get type by id
    virtual llvm::Type     *type    (const std::string &) = 0;

    /// Set value by id
    virtual void value   (const std::string &, llvm::Value *) = 0;
    /// Set function by id
    virtual void function(const std::string &, llvm::Function *) = 0;
    /// Set type by id
    virtual void type    (const std::string &, llvm::Type *) = 0;

    virtual ContextChild make_frame() = 0;

  protected:
    using ValueMap    = std::map<std::string, llvm::Value *>;
    using FunctionMap = std::map<std::string, llvm::Function *>;
    using TypeMap     = std::map<std::string, llvm::Type *>;
  };

  /// A subcontext of some parent Context.

  /// This holds a reference to its parent Context to do the lookups when
  /// something isn't found on the current Context
  class ContextChild : public Context {
  public:
    ContextChild(Context &parent);

    llvm::LLVMContext &llvm_ctx() override;
    llvm::IRBuilder<> &builder() override;
    llvm::Module      &module() override;

    bool has_value   (const std::string &) override;
    bool has_function(const std::string &) override;
    bool has_type    (const std::string &) override;

    llvm::Value    *value   (const std::string &) override;
    llvm::Function *function(const std::string &) override;
    llvm::Type     *type    (const std::string &) override;

    void value   (const std::string &, llvm::Value *) override;
    void function(const std::string &, llvm::Function *) override;
    void type    (const std::string &, llvm::Type *) override;

    ContextChild make_frame() override;

  private:
    Context &m_parent;

    ValueMap    m_values;
    FunctionMap m_functions;
    TypeMap     m_types;
  };


  /// The root of the Context tree.

  /// This Actually holds the llvm builder, context and
  /// module as well as the root symbol table
  class ContextRoot : public Context {
  public:
    // Context(llvm::LLVMContext &llvm_ctx,
    //         llvm::IRBuilder<> &builder,
    //         llvm::Module      &module,
    //         Context           &m_parent);
    ContextRoot(std::string module_name);

    llvm::LLVMContext &llvm_ctx() override;
    llvm::IRBuilder<> &builder() override;
    llvm::Module      &module() override;

    bool has_value   (const std::string &) override;
    bool has_function(const std::string &) override;
    bool has_type    (const std::string &) override;

    llvm::Value    *value   (const std::string &) override;
    llvm::Function *function(const std::string &) override;
    llvm::Type     *type    (const std::string &) override;

    void value   (const std::string &, llvm::Value *) override;
    void function(const std::string &, llvm::Function *) override;
    void type    (const std::string &, llvm::Type *) override;

    ContextChild make_frame() override;

  private:
    // TODO: should probably be a shared_ptr
    llvm::LLVMContext m_llvm_ctx;
    llvm::IRBuilder<> m_builder;
    llvm::Module      m_module;

    ValueMap    m_values;
    FunctionMap m_functions;
    TypeMap     m_types;
  };
}

#endif
