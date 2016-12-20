#include "big_int.hpp"

using alexstrong::big_int;

int main() {
  std::cout << "CHAR_BIT is " << CHAR_BIT << std::endl;
  big_int<CHAR_BIT> char1;
  big_int<2*CHAR_BIT> short1;
  auto sum = char1 + short1;
  //big_int<CHAR_BIT+1> char2;
  std::cout << sizeof(char1);
  std::cout << sizeof(short1);
  std::cout << sizeof(sum);
}
