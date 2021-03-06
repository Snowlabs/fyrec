#include <parser/parser.h>

#include "parser.h"
#include "AST.h"

#include <sstream>
#include <vector>
#include <memory>

using std::shared_ptr;
using std::make_shared;

using Parser::CharSet;
// Actual parsing

namespace Fyre {

  namespace Sets {
#define from_std(pred, desc)                                            \
    const CharSet pred = {desc, [](char c){ return (bool)std::pred(c); }};

    from_std(isupper, "upper-case")
    from_std(islower, "lower-case")
    from_std(isalnum, "alphanumeric")
    from_std(isdigit, "digit")

#undef from_std
  }

  // namespace FyreParser {

  namespace ExtraParsers {
    template<class Enc>
    struct Encloser {
      static const CharSet lhs;
      static const CharSet rhs;
    };

    template<class Parsable, class Enc>
    struct Enclosed {
      static auto parse(Parser::IParseStream &in) {
        in.begin_token();
        in.one_of(Encloser<Enc>::lhs);

        auto r = in.one_of<Parsable>();

        in.begin_token();
        in.one_of(Encloser<Enc>::rhs);
        return r;
      }
    };

    struct EParens;

    template<> const CharSet Encloser<EParens>::lhs = {'('};
    template<> const CharSet Encloser<EParens>::rhs = {')'};

    template<class P>
    using Parens = Enclosed<P, EParens>;

    template<class Sep>
    struct Separator {
      static const CharSet sep;
    };

    template<class Parsable, class Sep>
    struct SepBy {
      static auto parse(Parser::IParseStream &in) {
        std::vector<typename Parser::ParsableTrait<Parsable>::ResultType> r;

        // NOTE: accepts trailing separator
        for (;;) {
          auto e = in.maybe_of<Parsable>();
          if (!e)
            break;

          r.push_back(*e);

          auto s = in.maybe_of(Separator<Sep>::sep);
          if (!s)
            break;
        }

        return r;
      }
    };

    struct SComma;

    template<> const CharSet Separator<SComma>::sep = {','};

    struct TypeIdent {
      static std::string parse(Parser::IParseStream &in) {
        std::stringstream r;

        r << in.one_of ({Sets::isupper})
          << in.many_of({'_', Sets::isalnum});

        return r.str();
      }
    };

    struct TypeNoArgs {
      static TypePtr parse(Parser::IParseStream &in) {
        auto id = in.one_of<ExtraParsers::TypeIdent>();

        return make_shared<Type>(id);
      }
    };

    struct TypeArg {
      static TypePtr parse(Parser::IParseStream &in) {
        in.begin_token();
        in.one_of({'.'});

        auto typ = in.one_of<Parens<Type>, TypeNoArgs>();

        return typ;
      }
    };

    struct FunDecArg {
      static FunDec::Arg parse(Parser::IParseStream &in) {

        auto name = in.maybe_of<Ident>();
        auto type = in.one_of<Type>();

        return { name, type };
      }
    };

    struct FunDefArg {
      static FunDef::Arg parse(Parser::IParseStream &in) {

        auto name = in.one_of<Ident>();
        auto type = in.maybe_of<Type>();

        return { name, type };
      }
    };

    struct FunCalArg {
      static FunCal::Arg parse(Parser::IParseStream &in) {

        auto name = in.one_of<Expr>();

        return name;
      }
    };
  }

  IdentPtr Ident::parse(Parser::IParseStream &in) {
    std::stringstream r;

    in.begin_token();

    r << in.one_of ({'_', Sets::islower})
      << in.many_of({'_', Sets::isalnum});

    return make_shared<Ident>(r.str());
  }

  TypePtr Type::parse(Parser::IParseStream &in) {
    std::stringstream r;

    in.begin_token();

    // r << ExtraParsers::TypeIdent>::parse(in);
    r << in.one_of<ExtraParsers::TypeIdent>();

    std::vector<TypePtr> args =
      in.many_of<ExtraParsers::TypeArg>();

    return make_shared<Type>(r.str(), args);
  }

  ExprPtr Expr::parse(Parser::IParseStream &in) {

    return in.one_of_as<ExprPtr, FunCal, Var, IntLit>();

    // return in.one_of<FunCal, IntLit>();
  }

  IntLitPtr IntLit::parse(Parser::IParseStream &in) {
    std::stringstream r;
    in.begin_token();

    r << in.one_of ({Sets::isdigit})
      << in.many_of({Sets::isdigit});

    long long int v = std::stoi(r.str());
    return make_shared<IntLit>(v);
  }

  VarPtr Var::parse(Parser::IParseStream &in) {
    auto id = in.one_of<Ident>();

    return make_shared<Var>(id);
  }

  FunDecPtr FunDec::parse(Parser::IParseStream &in) {
    using namespace ExtraParsers;

    auto id   = in.one_of<Ident>();

    auto args = in.one_of<Parens<SepBy<FunDecArg, SComma>>>();

    auto type = in.one_of<Type>();

    return make_shared<FunDec>(id, args, type);
  }

  FunDefPtr FunDef::parse(Parser::IParseStream &in) {
    using namespace ExtraParsers;

    auto id   = in.one_of<Ident>();

    auto args = in.one_of<Parens<SepBy<FunDefArg, SComma>>>();

    auto type = in.one_of<Type>();

    in.begin_token();
    in.one_of({'='});

    auto expr = in.one_of<Expr>();

    return make_shared<FunDef>(id, args, type, std::nullopt, expr);
  }

  FunCalPtr FunCal::parse(Parser::IParseStream &in) {
    using namespace ExtraParsers;

    auto id   = in.one_of<Ident>();
    auto args = in.one_of<Parens<SepBy<FunCalArg, SComma>>>();

    return make_shared<FunCal>(id, args);
  }

  TopLvlPtr TopLvl::parse(Parser::IParseStream &in) {
    return in.one_of_as<TopLvlPtr, FunDef, FunDec>();
  }

  ModulePtr Module::parse(Parser::IParseStream &in) {
    std::vector<TopLvlPtr> r;

    in.skip_ws();
    while (in.peek() != EOF) {
      try {

        r.push_back(in.one_of<TopLvl>());

      } catch (Parser::Error e) {

        in.skip_ws();

        if(!in.maybe_of({';'}))
          throw e;

        in.skip_ws();
      }

    }

    return make_shared<Module>(r);
  }

}

