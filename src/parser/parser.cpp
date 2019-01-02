// #include "AST.h"
#include "exceptions.h"
#include "parser.h"

#include <regex>
#include <cctype>
#include <string>

namespace Parser {
  LocStreamBuf::LocStreamBuf(std::streambuf *sb,
                             std::string filename) : m_sb(sb) {
    m_buf = std::vector<char>();
    m_loc = { filename, 1, 0, 0 };
  }

  LocStreamBuf::~LocStreamBuf() {
    sync();
  }

  Location LocStreamBuf::get_loc() {
    if (m_loc.total_chr != (gptr() - eback())) {
      return upd_loc();
    }

    return m_loc;
  }


  Location LocStreamBuf::upd_loc() {
    std::streampos prev_pos = m_loc.total_chr;
    std::streampos curr_pos = (gptr() - eback());
    char *beg = eback();
    char *end = egptr();

    //TODO: if its been saved:
    //if its easier to count from beg:
    if (labs(curr_pos - prev_pos) > curr_pos) {
      m_loc.line = 1;
      m_loc.chr = 0;
      for (int i = 0; i < curr_pos; i++) {
        if (i > 0 && beg[i-1] == '\n') {
          m_loc.line++;
          m_loc.chr = 0;
        } else {
          m_loc.chr++;
        }
      }
    } else if (curr_pos > prev_pos) {
      //if its easier to count form curpos back/forward to pos:

      // count forwards to adjust location
      for (long i = prev_pos; i < curr_pos; i++) {
        if (beg[i] == '\n') {
          m_loc.line++;
          m_loc.chr = 0;
        } else {
          m_loc.chr++;
        }
      }
    } else {
      // count backwards to adjust
      for (long i = prev_pos; i > curr_pos; i--) {
        m_loc.total_chr--;
        if (i+1 < (end - beg) && beg[i+1] == '\n')
          m_loc.line--;
      }
      m_loc.chr = 0;
      for (long i = curr_pos; (i > 0) && !(i > 1 && beg[i-2] == '\n'); i--)
        m_loc.chr++;
    }

    m_loc.total_chr = curr_pos;

    return m_loc;
  }


  std::streambuf *LocStreamBuf::setbuf(char *s, std::streamsize n) {
    // return m_sb->pubsetbuf(s, n);
    return this;
  }

  std::streampos LocStreamBuf::seekoff(std::streamoff off,
                                       std::ios_base::seekdir way,
                                       std::ios_base::openmode which) {

    if (which == std::ios_base::out)
      return -1;

    off_type rel;
    char *beg = eback();
    char *cur = gptr();
    char *end = egptr();

    switch (way) {
    case std::ios_base::beg:
      rel = 0;
      break;

    case std::ios_base::cur:
      rel = cur - beg;
      break;

    case std::ios_base::end:
      rel = end - beg;
      break;

    default:
      return -1;
    }

    std::streampos pos = rel + off;

    return seekpos(pos, which);
  }

  std::streampos LocStreamBuf::seekpos(std::streampos pos,
                                       std::ios_base::openmode which) {
    if (which == std::ios_base::out)
      return -1;

    char *beg = eback();
    char *end = egptr();

    if (pos < 0 || pos > (end - beg))
      return -1;

    setg(beg, beg+pos, end);

    return pos;
  }

  // int LocStreamBuf::sync() {
  //   int r = 0;
  //   if (m_sb != NULL) {
  //     r = m_sb->sputbackc(*gptr());
  //     setg(NULL, NULL, NULL);
  //   }

  //   if (m_sb->pubsync() == EOF)
  //     r = EOF;

  //   return r;
  //   // return m_sb->pubsync();
  // }

  // std::streamsize LocStreamBuf::showmanyc() {
  //   return m_sb->in_avail();
  // }

  // int LocStreamBuf::sbumpc() {
  //   return m_sb->sbumpc();
  // }

  // std::streamsize LocStreamBuf::xsgetn(char* s, std::streamsize n) {
  //   return
  // }

  int LocStreamBuf::underflow() {
    int r = EOF;

    if (gptr() < egptr()) {
      r = *gptr();
    } else if (m_sb != NULL) {
      r = m_sb->sbumpc();
      if (r != EOF) {

        m_buf.push_back(r);

        char *beg = m_buf.data();
        size_t size = m_buf.size();

        setg(beg, beg+size-1, beg+size);
      }
    }


    return r;
  }

  int LocStreamBuf::uflow() {
    if ( underflow() == EOF ) return EOF;

    gbump(1);
    return gptr()[-1];
  }


  void CharSet::add(char c) {
    m_chars.push_back(c);
  }

  void CharSet::add(Predicate p) {
    m_preds.push_back(p);
  }

  void CharSet::add(DescPredicate dp) {
    add(dp.pred);
    m_pred_descs.push_back(dp.desc);
  }

  void CharSet::add(std::vector<char> cs) {
    m_chars.reserve(m_chars.capacity() + cs.size());
    for (auto c : cs)
      add(c);
    // m_chars.push_back(c);
  }

  void CharSet::add(std::vector<Predicate> ps) {
    m_preds.reserve(m_preds.capacity() + ps.size());
    for (auto p : ps)
      add(p);
    // m_preds.push_back(p);
  }

  void CharSet::add(CharSet s) {
    add(s.m_chars);
    add(s.m_preds);
  }

  // CharSet& CharSet::operator=(char c) {
  //   return operator=(CharSet(c));
  // }
  // CharSet& CharSet::operator=(const Predicate c) {
  //   return operator=(CharSet(c));
  // }

  bool CharSet::contains(char c) const {
    for (char mc : m_chars)
      if (c == mc)
        return true;

    for (auto mp : m_preds)
      if (mp(c))
        return true;

    return false;
  }


  // TODO: make this prettier
  std::string CharSet::to_string() const {
    std::stringstream r;

    // r << "{ ";
    bool first = true;
    for (auto &c : m_chars) {
      if (!first) r << ", ";
      else first = false;

      r << "`" << c << "`";
    }

    if (!first) r << ", ";
    first = true;
    for (auto &p : m_pred_descs) {
      if (!first) r << ", ";
      else first = false;

      r << p;
    }
    // r << " }";

    return r.str();
  }



  IParseStream::IParseStream(const std::istream &is) {
    rdbuf(new LocStreamBuf(is.rdbuf()));
    // rdbuf(is.rdbuf());
  }

  IParseStream::~IParseStream() {
    delete rdbuf();
  }

  LocStreamBuf *IParseStream::lrdbuf() {
    return (LocStreamBuf *)rdbuf();
  }

  Location IParseStream::get_loc() {
    return lrdbuf()->get_loc();
  }

  std::string IParseStream::get_while(std::function<bool(char)> pred) {
    std::string r;

    while (pred(peek()))
      r += get();

    return r;
  }

  std::string IParseStream::many_of(const CharSet &set) {
    std::string r;

    while (set.contains(peek()))
      r += get();

    return r;
  }

  // template<class Fn,
  //          class R>
  //          // class R = decltype(std::invoke(std::declval<Fn>(),
  //          //                                std::declval<IParseStream&>()))>
  char IParseStream::one_of(const CharSet &set) {
    if (set.contains(peek()))
      return get();

    throw Error(get_loc(),
                "Expected character from: "+set.to_string()+
                "\nGot: '"+(char)peek()+"'");
  }

  std::optional<char> IParseStream::maybe_of(const CharSet &set) {
    try {
      return one_of(set);
    } catch (Error) {
      return std::nullopt;
    }
  }

  void IParseStream::skip_ws() {
    while(std::isspace(peek()))
      get();
  }

  void IParseStream::throw_on_eof() {
    if (peek() == EOF)
      throw UnexpectedEOF(get_loc());
  }

  IParseStream &IParseStream::begin_token() {
    skip_ws();
    throw_on_eof();

    return *this;
  }

  // template<class T>
  // // template<class Fn, class typename Parser<T>::is_parser<Fn>>
  // template<class Fn>//, class>
  // template<class Fn>
  // Parser<Fn>::Parser(Fn f, std::optional<std::string> d)
  //   : m_func(f), m_desc(d) {}

  // template<class T>
  // T Parser<T>::parse(IParseStream &in) {
  //   return m_func(in);
  // }

  // template<class T>
  // T Parser<T>::try_parse(IParseStream &in) {
  //   auto p = in.tellg();
  //   try {
  //     T r = parse(in);
  //     return r;

  //   } catch (Error e) {
  //     in.seekg(p);
  //     throw e;
  //   }
  // }

  // template<class T>
  // std::string Parser<T>::to_string() const {
  //   return m_desc;
  // }

  
}
