#include "big_int.hpp"
#include <cassert>
#include <iostream>
#include <typeinfo>

using namespace alexstrong;

int main() {
  big_int<2*CHAR_BIT> char1("252");
  std::cout << char1 << std::endl;
  //big_int<CHAR_BIT> char2(char1);
  //assert(char1 == char2);
  //std::cout << char1 << std::endl;
  return 0;
}
