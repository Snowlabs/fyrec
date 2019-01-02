#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <istream>
#include <ios>
#include <locale>
#include <vector>
#include <functional>
#include <variant>
#include <initializer_list>
#include <type_traits>

#include "location.h"
#include "exceptions.h"

namespace Parser {

  template<class Parsable>
  class Parser;

  class IParseStream;

  // NOTE: this might be better done by extending stringbuf
  class LocStreamBuf : public std::streambuf {
  public:
    LocStreamBuf(std::streambuf *sb,
                 std::string filename = "-");
    ~LocStreamBuf() override;

    Location get_loc();
    Location upd_loc();

  private:
    std::streambuf *m_sb;
    std::vector<char> m_buf;
    Location m_loc;

  protected:
    std::streambuf *setbuf(char *s, std::streamsize n) override;
    std::streampos seekoff(std::streamoff off,
                           std::ios_base::seekdir way,
                           std::ios_base::openmode which =
                           std::ios_base::in | std::ios_base::out) override;
    std::streampos seekpos(std::streampos sp,
                           std::ios_base::openmode which =
                           std::ios_base::in | std::ios_base::out) override;
    // int sync() override;

    // std::streamsize showmanyc();
    //  std::streamsize xsgetn(char* s, std::streamsize n);
    int underflow() override;

    int uflow() override;
  };


  template<class T>
  struct ParsableTrait {
    using ResultType = std::invoke_result_t<decltype(T::parse), IParseStream&>;

    using ParserType = Parser<ResultType>;

    static auto parse(IParseStream &in) {
      return T::parse(in);
    }
    static const Parser<ResultType> parser;
  };

  template<class T>
  const typename ParsableTrait<T>::ParserType ParsableTrait<T>::parser =
    Parser(T::parse);

  class CharSet {
  public:
    typedef std::function<bool(char)> Predicate;

    template<class Fn>
    using is_predicate_t = std::is_invocable_r<bool, Fn, char>;

    template<class Fn>
    using is_predicate = std::enable_if_t<is_predicate_t<Fn>::value>;

    typedef struct {
      std::string desc;
      Predicate   pred;
    } DescPredicate;

    CharSet() {};

#define ctor(typ)                                         \
    template<class ...T>                                  \
    CharSet(typ c0, T... c) : CharSet(c...) { add(c0); }

    ctor(char)
    ctor(DescPredicate)
    ctor(Predicate)
    ctor(CharSet)
#undef ctor

    template<class Fn, class = is_predicate<Fn>>
    CharSet(std::string d, Fn p) : CharSet(DescPredicate{d, p}) {}

    template<class Fn, class = is_predicate<Fn>>
    CharSet(Fn p) : CharSet(Predicate(p)) {}


    void add(char);
    void add(Predicate);
    void add(DescPredicate);
    void add(std::vector<char>);
    void add(std::vector<Predicate>);
    void add(CharSet);

    bool contains(char) const;

    std::string to_string() const;

  private:
    std::vector<char>        m_chars;
    std::vector<Predicate>   m_preds;
    std::vector<std::string> m_pred_descs;
  };



  class IParseStream : public std::istream {
  public:
    IParseStream(const std::istream &is);
    ~IParseStream();

    LocStreamBuf *lrdbuf();

    Location get_loc();

    std::string get_while(const std::function<bool(char)> pred);


    std::string many_of(const CharSet &set);

    // template<class T>
    // using IsParsableTrait = std::is_same<ParsableTrait<T>, T>;

    // template<class T>
    // using ToParsableTrait = std::conditional<IsParsableTrait<T>::value,
    //                                          T,
    //                                          ParsableTrait<T>>;

    template<class ParsableTr>
    std::vector<typename ParsableTrait<ParsableTr>::ResultType> many_of() {
      std::vector<typename ParsableTrait<ParsableTr>::ResultType> r;

      for (;;) {
        auto p = tellg();
        try {
          auto rr = ParsableTrait<ParsableTr>::parser.parse(*this);
          r.push_back(rr);

        } catch (Error e) {
          seekg(p);
          break;
          // throw e;
        }
      }

      return r;
    }

    char one_of(const CharSet &set);

    template<class ParsableTr>
    // Parsable try_parse(const Parser<Parsable> p) {
    typename ParsableTrait<ParsableTr>::ResultType one_of() {
      auto pos = tellg();
      try {
        return ParsableTrait<ParsableTr>::parser.parse(*this);

      } catch (Error e) {
        seekg(pos);

        throw e;
      }
    }

    template<class ParsableTr, class ParsableTr1, class ...Rest>
    typename ParsableTrait<ParsableTr>::ResultType one_of() {
      auto pos = tellg();
      try {
        return ParsableTrait<ParsableTr>::parser.parse(*this);

      } catch (Error e) {
        seekg(pos);

        return one_of<ParsableTr1, Rest...>();
        // throw e;
      }
      // return p.try_parse(*this);
    }


    std::optional<char> maybe_of(const CharSet &set);

    template<class ParsableTr, class ...ParsableTrs>
    std::optional<typename ParsableTrait<ParsableTr>::ResultType> maybe_of() {
      try {
        return one_of<ParsableTr, ParsableTrs...>();
      } catch (Error) {
        return std::nullopt;
      }
    }


    void skip_ws();
    void throw_on_eof();
    IParseStream& begin_token();
  };


  template<class ParsableT>
  class Parser {
  public:
    typedef ParsableT Parsable;

    typedef std::function<Parsable(IParseStream &)> PFunc;

    template<class Fn>
    Parser(Fn f, std::optional<std::string> d = std::nullopt)
      : m_func(f), m_desc(d) {}


    std::string to_string() const;

    friend IParseStream;

  private:
    Parsable parse(IParseStream &in) const {
      return m_func(in);
    }

    PFunc m_func;
    std::optional<std::string> m_desc;

  };

  template<class Fn>
  Parser(Fn, std::optional<std::string> = std::nullopt)
    -> Parser<decltype(std::invoke(std::declval<Fn>(),
                                   std::declval<IParseStream&>()))>;


  template<class Parsable>
  class ParserSet {
  public:
    // typedef std::function<Parsable(IParseStream &)> Parser;

    // template<class Fn>
    // using is_parser_t = std::is_invocable_r<Parsable, Fn, IParseStream &>;

    // template<class Fn>
    // using is_parser = std::enable_if_t<is_parser_t<Fn>::value>;


    ParserSet() {};

#define ctor(typ)                                             \
    template<class ...T>                                      \
    ParserSet(typ c0, T... c) : ParserSet(c...) { add(c0); }

    ctor(typename Parser<Parsable>::PFunc)
    ctor(ParserSet)
#undef ctor

    // template<class Fn, class = is_parser<Fn>>
    // ParserSet(std::string d, Fn p) : ParserSet(DescParser{d, p}) {}

    // template<class Fn, class = is_parser<Fn>>
    // ParserSet(Fn p) : ParserSet(Predicate(p)) {}


    void add(Parser<Parsable>);
    void add(ParserSet<Parsable>);

    std::string to_string() const;

  private:
    std::vector<Parser<Parsable>> m_parsers;
  };

  
}
#endif
