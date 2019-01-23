#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <memory>
#include <type_traits>
#include <variant>

#include <parser/location.h>
#include <parser/parser.h>

#include "context.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace Fyre {

  // I know this isnt pretty, but cpp is too verbose
#define decl_ptr(Typ) \
  class Typ;                                 \
  using Typ##Ptr = std::shared_ptr<Typ>;

  decl_ptr(ANode);
  decl_ptr(TopLvl);
  decl_ptr(Ident);
  decl_ptr(Type);
  decl_ptr(Expr);
  decl_ptr(IntLit);
  decl_ptr(Var);
  decl_ptr(FunDec);
  decl_ptr(FunDef);
  decl_ptr(FunCal);
  decl_ptr(Module);

#undef decl_ptr

  // template<class T>
  // struct ParsableAST {
  //   static ANodeP parse(Parser::Parser::IParseStream &);
  // };

  // template<class T>
  // struct ParsableTrait {
  //   using ResultType = std::invoke_result_t<decltype(T::parse), Parser::IParseStream&>;

  //   using ParserType = Parser<ResultType>;

  //   static auto parse(Parser::IParseStream &in) {
  //     return T::parse(in);
  //   }
  //   static const Parser<ResultType> parser;
  // };

  // template<class T>
  // const typename ParsableTrait<T>::ParserType ParsableTrait<T>::parser =
  //   Parser(T::parse);


  class ANode {
  public:
    virtual ~ANode() {};


    virtual std::string to_string() const = 0;

  protected:
    Parser::Location *m_loc;
    bool m_resolved;
  };

  class TopLvl : public ANode {
  public:
    using StatementIR = std::variant<llvm::Function *>;

    static TopLvlPtr parse(Parser::IParseStream &);

    virtual StatementIR codegen(Context &ctx) const = 0;
  };

  class Ident : public ANode {
  public:
    Ident(std::string ident);

    std::string to_string() const;

    static IdentPtr parse(Parser::IParseStream &);

    std::string str() const;
    // using PTrait = Parser::ParsableTrait<ParsableAST<Ident>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    std::string m_str;
  };

  class Type : public ANode {
  public:
    Type(std::string type, std::vector<TypePtr> args = {});

    std::string to_string() const;

    static TypePtr parse(Parser::IParseStream &);

    llvm::Type *codegen(Context &ctx) const;
    // using PTrait = Parser::ParsableTrait<ParsableAST<Type>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    std::string m_str;
    std::vector<TypePtr> m_args;
  };

  class Expr : public ANode {
  public:
    // std::string to_string() const;

    static ExprPtr parse(Parser::IParseStream &);

    virtual llvm::Value *codegen(Context &ctx) const = 0;
    // using PTrait = Parser::ParsableTrait<ParsableAST<Expr>>;
    // static const Parser::Parser<ANodeP> parser;
  };

  class IntLit : public Expr {
  public:
    IntLit(long long int val = 0);
    std::string to_string() const;

    static IntLitPtr parse(Parser::IParseStream &);

    llvm::Value *codegen(Context &ctx) const;
    // using PTrait = Parser::ParsableTrait<ParsableAST<Expr>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    long long int m_val;
  };

  class Var : public Expr {
  public:
    Var(IdentPtr name);
    std::string to_string() const;

    static VarPtr parse(Parser::IParseStream &);

    llvm::Value *codegen(Context &ctx) const;

  protected:
    IdentPtr m_name;
  };

  class FunDec : public TopLvl {
  public:
    struct Arg {
      std::optional<IdentPtr> name;
      TypePtr type;
    };

    // TODO: implement contexts
    FunDec(IdentPtr name, std::vector<Arg> args,
           TypePtr  type, std::optional<TypePtr> context = std::nullopt);

    std::string to_string() const;
    std::string name_str() const;

    static FunDecPtr parse(Parser::IParseStream &);

    TopLvl::StatementIR codegen(Context &ctx) const;
    // using PTrait = Parser::ParsableTrait<ParsableAST<FunDec>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    IdentPtr m_name;
    std::vector<Arg> m_args;
    TypePtr m_type;
    std::optional<TypePtr> m_context;
  };
  std::ostream &operator<<(std::ostream &os, FunDec::Arg const &arg);

  class FunDef : public TopLvl {
  public:
    struct Arg {
      IdentPtr name;
      std::optional<TypePtr> type;
    };

    // FunDef(ANodeP name, std::vector<Arg> args,
    //        ANodeP type,
    //        ANodeP expr);
    FunDef(IdentPtr name, std::vector<Arg> args,
           TypePtr  type, std::optional<TypePtr> context,
           ExprPtr  expr);

    std::string to_string() const;
    std::string name_str() const;

    static FunDefPtr parse(Parser::IParseStream &);

    TopLvl::StatementIR codegen(Context &ctx) const;
    // using PTrait = Parser::ParsableTrait<ParsableAST<FunDec>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    IdentPtr m_name;
    std::vector<Arg> m_args;
    TypePtr m_type;
    std::optional<TypePtr> m_context;
    ExprPtr m_expr;
  };
  std::ostream &operator<<(std::ostream &os, FunDec::Arg const &arg);

  class FunCal : public Expr {
  public:
    using Arg = ExprPtr;

    FunCal(IdentPtr name, std::vector<Arg> args);

    std::string to_string() const;

    static FunCalPtr parse(Parser::IParseStream &);

    llvm::Value *codegen(Context &ctx) const;
    // using PTrait = Parser::ParsableTrait<ParsableAST<FunDec>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    IdentPtr m_name;
    std::vector<Arg> m_args;
  };


  class Module : public ANode {
  public:
    Module(std::vector<TopLvlPtr>);

    std::string to_string() const;

    static ModulePtr parse(Parser::IParseStream &);

    // std::unique_ptr<llvm::Module> codegen() const;
    std::unique_ptr<ContextRoot> codegen(const std::string &) const;

  protected:
    std::vector<TopLvlPtr> m_statements;
  };


  std::ostream &operator<<(std::ostream &os, ANode const &id);

  template<class T>
  std::ostream &operator<<(std::ostream &os, std::shared_ptr<T> const &id) {
    return os << *static_cast<ANodePtr>(id);
  }
}

#endif
