#include <sstream>

#include "AST.h"

namespace Fyre {
  std::ostream &operator<<(std::ostream &os, ANode const &id) {
    return os << id.to_string();
  }


  Ident::Ident(std::string ident) : m_str(ident) {}

  std::string Ident::to_string() const { return m_str; }
  std::string Ident::str() const { return to_string(); }

  Type::Type(std::string type, std::vector<TypePtr> args)
    : m_str(type), m_args(args) {}

  std::string Type::to_string() const {
    std::stringstream r;

    r << m_str;
    for (auto &arg : m_args)
      r << ".(" << arg << ")";

    return r.str();
  }

  IntLit::IntLit(long long int val) : m_val(val) {}
  std::string IntLit::to_string() const {
    return std::to_string(m_val);
  }

  Var::Var(IdentPtr name) : m_name(name) {}
  std::string Var::to_string() const {
    return m_name->to_string();
  }

  FunDec::FunDec(IdentPtr name, std::vector<Arg> args,
                 TypePtr type, std::optional<TypePtr> context)
    : m_name(name), m_args(args),
      m_type(type), m_context(context) {}

  std::ostream &operator<<(std::ostream &os, FunDec::Arg const &arg) {
    if (arg.name)
      os << *arg.name << " ";
    os << arg.type;
    return os;
  }

  std::string FunDec::to_string() const {
    std::stringstream r;

    r << m_name
      << "(";

    for (auto &arg : m_args) {
      r << arg;
      if (&arg < &m_args.back())
        r << ", ";
    }

    r << ") "
      << m_type;

    if (m_context)
      r << " : "
        << *m_context;

    return r.str();
  }

  std::string FunDec::name_str() const {
    return m_name->str();
  }

  FunDef::FunDef(IdentPtr name, std::vector<Arg> args,
                 TypePtr type, std::optional<TypePtr> context,
                 ExprPtr expr)
    : m_name(name), m_args(args),
      m_type(type), m_context(context),
      m_expr(expr) {}

  std::ostream &operator<<(std::ostream &os, FunDef::Arg const &arg) {
    os << arg.name;

    if (arg.type)
      os << " " << *arg.type;

    return os;
  }

  std::string FunDef::to_string() const {
    std::stringstream r;

    r << m_name
      << "(";

    for (auto &arg : m_args) {
      r << arg;
    if (&arg < &m_args.back())
      r << ", ";
    }

    r << ") "
      << m_type;

    if (m_context)
      r << " : "
        << *m_context;

    r << " = "
      << m_expr;

    return r.str();
  }

  std::string FunDef::name_str() const {
    return m_name->str();
  }

  FunCal::FunCal(IdentPtr name, std::vector<Arg> args)
    : m_name(name), m_args(args) {}

  std::string FunCal::to_string() const {
    std::stringstream r;

    r << m_name
      << "(";

    for (auto &arg : m_args) {
      r << arg;
      if (&arg < &m_args.back())
        r << ", ";
    }

    r << ")";

    return r.str();
  }

  Module::Module(std::vector<TopLvlPtr> stmnts) : m_statements(stmnts) {}

  std::string Module::to_string() const {
    std::stringstream r;

    for (auto &statement : m_statements) {
      r << statement->to_string() << ";\n";
    }

    return r.str();
  }

  // const auto Ident::parser  = Ident::PTrait::parser;
  // const auto Type::parser   = Type::PTrait::parser;
  // const auto Expr::parser   = Expr::PTrait::parser;
  // const auto IntLit::parser = IntLit::PTrait::parser;
  // const auto FunDec::parser = FunDec::PTrait::parser;

}
