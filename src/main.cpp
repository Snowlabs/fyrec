
#include <iostream>
#include <sstream>
#include <cassert>

#include "fyre/AST.h"
#include "fyre/parser.h"

#include "parser/parser.h"
#include "parser/exceptions.h"

using std::cout;
using std::endl;

// TODO: move this to better place (at least in the proper dir/ns)
void test_loc() {
  std::stringstream ss("012345\n789012\n");
  Parser::IParseStream in(ss);
  Parser::Location l;

  // Initial
  l = in.get_loc();
  assert(l.line == 1);
  assert(l.chr  == 0);
  assert(l.total_chr == 0);

  in.get();
  // cout << (char)in.get() << endl;

  // Forward
  l = in.get_loc();
  assert(l.line == 1);
  assert(l.chr  == 1);
  assert(l.total_chr == 1);

  auto pos = in.tellg();
  for (auto i = 0; i < 4; i++)
    in.get();

  // Forward
  l = in.get_loc();
  assert(l.line == 1);
  assert(l.chr  == 5);
  assert(l.total_chr == 5);

  in.seekg(pos);

  // Reverse
  l = in.get_loc();
  assert(l.line == 1);
  assert(l.chr  == 1);
  assert(l.total_chr == 1);

  pos = in.tellg();
  for (auto i = 0; i < 9; i++)
    in.get();

  // Forward
  l = in.get_loc();
  assert(l.line == 2);
  assert(l.chr  == 3);
  assert(l.total_chr == 10);

  in.seekg(pos);

  // Reverse
  l = in.get_loc();
  assert(l.line == 1);
  assert(l.chr  == 1);
  assert(l.total_chr == 1);
}

int main(int argc, char **argv) {
  std::cout << "Hellow, olrd!" << std::endl;

  // test_loc();

  Parser::IParseStream in(std::cin);

  try {
    while(1) {
      Fyre::ANodeP ast = in.one_of<Fyre::FunDef>();

      in.begin_token();
      in.one_of({';'});

      std::cout << "fundef: " << ast << std::endl;
    }

  } catch (Parser::Error &e) {
    std::cerr << "Parser error: " << e.what() << std::endl;
  }



}
