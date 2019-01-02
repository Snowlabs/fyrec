#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <memory>
#include <type_traits>

#include <parser/location.h>
#include <parser/parser.h>

namespace Fyre {
  class ANode;
  typedef std::shared_ptr<ANode> ANodeP;

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

  class Ident : public ANode {
  public:
    Ident(std::string ident);

    std::string to_string() const;

    static ANodeP parse(Parser::IParseStream &);
    // using PTrait = Parser::ParsableTrait<ParsableAST<Ident>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    std::string m_str;
  };

  class Type : public ANode {
  public:
    Type(std::string type, std::vector<ANodeP> args = {});

    std::string to_string() const;

    static ANodeP parse(Parser::IParseStream &);
    // using PTrait = Parser::ParsableTrait<ParsableAST<Type>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    std::string m_str;
    std::vector<ANodeP> m_args;
  };

  class Expr : public ANode {
  public:
    // std::string to_string() const;

    static ANodeP parse(Parser::IParseStream &);
    // using PTrait = Parser::ParsableTrait<ParsableAST<Expr>>;
    // static const Parser::Parser<ANodeP> parser;
  };

  class IntLit : public Expr {
  public:
    IntLit(long long int val = 0);
    std::string to_string() const;

    static ANodeP parse(Parser::IParseStream &);
    // using PTrait = Parser::ParsableTrait<ParsableAST<Expr>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    long long int m_val;
  };

  class FunDec : public ANode {
  public:
    struct Arg {
      std::optional<ANodeP> name;
      ANodeP type;
    };

    FunDec(ANodeP name, std::vector<Arg> args,
           ANodeP type, std::optional<ANodeP> context = std::nullopt);

    std::string to_string() const;

    static ANodeP parse(Parser::IParseStream &);
    // using PTrait = Parser::ParsableTrait<ParsableAST<FunDec>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    ANodeP m_name;
    std::vector<Arg> m_args;
    ANodeP m_type;
    std::optional<ANodeP> m_context;
  };
  std::ostream &operator<<(std::ostream &os, FunDec::Arg const &arg);

  class FunDef : public ANode {
  public:
    struct Arg {
      ANodeP name;
      std::optional<ANodeP> type;
    };

    // FunDef(ANodeP name, std::vector<Arg> args,
    //        ANodeP type,
    //        ANodeP expr);
    FunDef(ANodeP name, std::vector<Arg> args,
           ANodeP type, std::optional<ANodeP> context,
           ANodeP expr);

    std::string to_string() const;

    static ANodeP parse(Parser::IParseStream &);
    // using PTrait = Parser::ParsableTrait<ParsableAST<FunDec>>;
    // static const Parser::Parser<ANodeP> parser;

  protected:
    ANodeP m_name;
    std::vector<Arg> m_args;
    ANodeP m_type;
    std::optional<ANodeP> m_context;
    ANodeP m_expr;
  };
  std::ostream &operator<<(std::ostream &os, FunDec::Arg const &arg);

  std::ostream &operator<<(std::ostream &os, ANode const &id);
  std::ostream &operator<<(std::ostream &os, ANodeP const &id);

}
#endif
