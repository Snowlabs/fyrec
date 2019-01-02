#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>
#include <optional>
#include "location.h"

namespace Parser {

  class Error : public std::exception {
  public:
    Error(std::optional<Location> loc = std::nullopt,
                std::optional<std::string> msg = std::nullopt);

    virtual const char *what() const throw();

  protected:
    std::string m_msg;
    std::optional<Location> m_loc;
  };

  class UnexpectedEOF : public Error {
  public:
    UnexpectedEOF(std::optional<Location> loc = std::nullopt);
  };

}


#endif
