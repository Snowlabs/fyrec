#ifndef LOCATION_H
#define LOCATION_H

#include <string>

namespace Parser {

  typedef struct {
    std::string file;

    unsigned int line;
    unsigned int chr;
    unsigned int total_chr;
  } Location;

}


#endif
