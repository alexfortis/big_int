#include "big_int.cpp"

int main() {
  std::cout << "CHAR_BIT is " << CHAR_BIT << std::endl;
  big_int<CHAR_BIT> char1;
  //big_int<CHAR_BIT+1> char2;
  std::cout << sizeof(char1);
}
