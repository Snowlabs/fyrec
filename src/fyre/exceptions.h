#ifndef COMPILER_EXCEPTIONS_H
#define COMPILER_EXCEPTIONS_H

#include <exception>
#include <string>
#include <optional>

namespace Compiler {

  // TODO: Add location info to all compiler errors
  class Error : public std::exception {
  public:
    Error(std::optional<std::string> msg = std::nullopt);

    virtual const char *what() const throw();

  protected:
    std::string m_msg;
  };

}


#endif
