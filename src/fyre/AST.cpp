#include <sstream>

#include "AST.h"

namespace Fyre {
  std::ostream &operator<<(std::ostream &os, ANode const &id) {
    return os << id.to_string();
  }

  std::ostream &operator<<(std::ostream &os, ANodeP const &id) {
    return os << id->to_string();
  }

  Ident::Ident(std::string ident) : m_str(ident) {}

  std::string Ident::to_string() const { return m_str; }

  Type::Type(std::string type, std::vector<ANodeP> args)
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

  FunDec::FunDec(ANodeP name, std::vector<Arg> args,
                 ANodeP type, std::optional<ANodeP> context)
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

  FunDef::FunDef(ANodeP name, std::vector<Arg> args,
                 ANodeP type, std::optional<ANodeP> context,
                 ANodeP expr)
    : m_name(name), m_args(args),
      m_type(type), m_context(context),
      m_expr(expr) {}

  std::ostream &operator<<(std::ostream &os, FunDef::Arg const &arg) {
    os << arg.name << " ";

    if (arg.type)
      os << *arg.type;

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
  // const auto Ident::parser  = Ident::PTrait::parser;
  // const auto Type::parser   = Type::PTrait::parser;
  // const auto Expr::parser   = Expr::PTrait::parser;
  // const auto IntLit::parser = IntLit::PTrait::parser;
  // const auto FunDec::parser = FunDec::PTrait::parser;

}
