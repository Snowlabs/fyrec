#include "context.h"

namespace Fyre {
  ContextRoot::ContextRoot(std::string module_name) :
    m_builder(m_llvm_ctx),
    m_module(llvm::Module(module_name, m_llvm_ctx)) {}

  llvm::LLVMContext &ContextRoot::llvm_ctx() {
    return m_llvm_ctx;
  }

  llvm::IRBuilder<> &ContextRoot::builder() {
    return m_builder;
  }
  llvm::Module &ContextRoot::module() {
    return m_module;
  }

  bool ContextRoot::has_value(const std::string &id) {
    if (m_values.count(id))
      return true;
    else
      return false;
  }
  bool ContextRoot::has_function(const std::string &id) {
    if (m_functions.count(id))
      return true;
    else
      return false;
  }
  bool ContextRoot::has_type(const std::string &id) {
    if (m_types.count(id))
      return true;
    else
      return false;
  }

  llvm::Value *ContextRoot::value(const std::string &id) {
    if (has_value(id))
      return m_values[id];
    else
      throw std::out_of_range("Value not found for id: " + id);
  }
  llvm::Function *ContextRoot::function(const std::string &id) {
    if (has_function(id))
      return m_functions[id];
    else
      throw std::out_of_range("Function not found for id: " + id);
  }
  llvm::Type *ContextRoot::type(const std::string &id) {
    if (has_type(id))
      return m_types[id];
    else
      throw std::out_of_range("Type not found for id: " + id);
  }

  void ContextRoot::value(const std::string &id, llvm::Value *v) {
    m_values[id] = v;
  }
  void ContextRoot::function(const std::string &id, llvm::Function *f) {
    m_functions[id] = f;
  }
  void ContextRoot::type(const std::string &id, llvm::Type *t) {
    m_types[id] = t;
  }

  ContextChild ContextRoot::make_frame() {
    return ContextChild(*this);
  }

  // -- ContextChild --

  ContextChild::ContextChild(Context &parent) : m_parent(parent) {}

  llvm::LLVMContext &ContextChild::llvm_ctx() {
    return m_parent.llvm_ctx();
  }
  llvm::IRBuilder<> &ContextChild::builder() {
    return m_parent.builder();
  }
  llvm::Module &ContextChild::module() {
    return m_parent.module();
  }

  bool ContextChild::has_value (const std::string &id) {
    if (m_values.count(id))
      return true;
    else
      return m_parent.has_value(id);
  }
  bool ContextChild::has_function(const std::string &id) {
    if (m_functions.count(id))
      return true;
    else
      return m_parent.has_function(id);
  }
  bool ContextChild::has_type(const std::string &id) {
    if (m_types.count(id))
      return true;
    else
      return m_parent.has_type(id);
  }

  llvm::Value *ContextChild::value(const std::string &id) {
    if (m_values.count(id))
      return m_values[id];
    else
      return m_parent.value(id);
  }
  llvm::Function *ContextChild::function(const std::string &id) {
    if (m_functions.count(id))
      return m_functions[id];
    else
      return m_parent.function(id);
  }
  llvm::Type *ContextChild::type(const std::string &id) {
    if (m_types.count(id))
      return m_types[id];
    else
      return m_parent.type(id);
  }

  void ContextChild::value(const std::string &id, llvm::Value *v) {
    m_values[id] = v;
  }
  void ContextChild::function(const std::string &id, llvm::Function *f) {
    m_functions[id] = f;
  }
  void ContextChild::type(const std::string &id, llvm::Type *t) {
    m_types[id] = t;
  }

  ContextChild ContextChild::make_frame() {
    return ContextChild(*this);
  }
}
