#include "exceptions.h"

#include <string>
#include <sstream>

namespace Compiler {
  Error::Error(std::optional<std::string> msg) {
    std::ostringstream r;
    r << "error compiling program";

    if (msg) {
      r << " : " << *msg;
    }

    m_msg = r.str();
  }

  const char *Error::what() const throw() {
    return m_msg.c_str();
  }
}
