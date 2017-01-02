#include "big_int.hpp"
#include <cassert>
#include <iostream>
#include <typeinfo>

using namespace alexstrong;

static const std::string first_b11("3AAA81234");
static const std::string first_b16("331AC83B");
static const std::string first_b10("857393211");
static const std::string first_b2("00110011000110101100100000111011");
static const int first_int = 857393211;

int main() {
  constexpr int int_bits = sizeof(int)*CHAR_BIT;
  std::cout << "Testing construction from string and equality for the size of an int." << std::endl;
  big_int<int_bits> x1(first_b11, 11);
  int x = x1.to_int();
  std::cout << x << std::endl;
  std::cout << x1 << std::endl;
  assert(x == first_int);
  big_int<int_bits> x2(first_b16, 16);
  assert(x1 == x2);
  std::cout << "Now testing for bigger integers." << std::endl;
  big_int<int_bits*2> x3(first_b16, 16);
  std::cout << "constructed" << std::endl;
  x3 *= x1;
  std::cout << "multiplication done" << std::endl;
  std::cout << x3 << std::endl;
  return 0;
}
