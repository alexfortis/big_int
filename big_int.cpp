#include <iostream>
#include <climits>

#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define STRINGIFY_HELPER(x) #x

template<int N>
class big_int {
  static_assert(N % CHAR_BIT == 0, "Invalid number of bits; must be a multiple of " STRINGIFY(CHAR_BIT));
  char bitmap[N/CHAR_BIT];

  //default constructor
  big_int() {
    for(int i = 0; i < N/CHAR_BIT; i++) {
      bitmap[i] = 0;
    }
  }

  //string constructor
  big_int(std::string value) {
    //TODO figure this out
  }

  //copy constructor
  big_int(const big_int<N> &other) {
    for(int i = 0; i < N/CHAR_BIT; i++) {
      bitmap[i] = other.bitmap[i];
    }
  }

  //move constructor
  big_int(big_int<N> &&other) {
    for(int i = 0; i < N/CHAR_BIT; i++) {
      bitmap[i] = other.bitmap[i];
      other.bitmap[i] = 0;
    }
  }

  //destructor
  ~big_int() {
    //do nothing, no pointers to free/delete
  }

  //copy assignment
  big_int<N> &operator=(const big_int<N> &other) {
    big_int<N> copy(other);
    return copy;
  }

  //move assignment
  big_int<N> &operator=(const big_int<N> &&other) {
    big_int<N> copy(other);
    return copy;
  }

  //define other operators here
  
};
