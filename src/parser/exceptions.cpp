#include "exceptions.h"
#include "parser.h"

#include <string>
#include <sstream>

namespace Parser {

  std::string indent(std::string in) {
    std::string r = "\t";

    for (char c : in) {
      if (c == '\n')
        r+= "\n\t";
      else
        r += c;
    }
    return r;
  }

  //TODO: make an error that says not only what expected but what got

  Error::Error(std::optional<Location>   loc,
                           std::optional<std::string> msg) : m_loc(loc) {
    std::ostringstream r;
    r << "error parsing input in ";

    // if location is known
    if (m_loc) {
      r << m_loc->file
        << " at "
        << m_loc->line << ":" << m_loc->chr;
    } else {
      r << "unknown location";
    }

    if (msg) {
      r << " : \n" << indent(*msg);
    }

    m_msg = r.str();
  }

  const char *Error::what() const throw() {
    return m_msg.c_str();
  }

  UnexpectedEOF::UnexpectedEOF(std::optional<Location> loc) {
    m_loc = loc;
    std::ostringstream r;
    r << "Unexpected end of file";

    // if location is known
    if (m_loc) {
      r << " in "
        << m_loc->file
        << " at "
        << m_loc->line << ":" << m_loc->chr;
    }

    m_msg = r.str();
  }

}
