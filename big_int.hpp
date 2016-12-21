#include <iostream>
#include <climits>

#ifndef BIG_INT_H
#define BIG_INT_H

#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define STRINGIFY_HELPER(x) #x

namespace alexstrong {

  template<int A, int B>
  struct IntUtils {
    static constexpr bool less = A<B;
    static constexpr bool greater = A>B;
    static constexpr bool equal = A==B;
    static constexpr int min = less ? A : B;
    static constexpr int max = greater ? A : B;
  };

  template<int N>
  class big_int {
    static_assert(N % CHAR_BIT == 0, "Invalid number of bits; " STRINGIFY(N) " is not a multiple of " STRINGIFY(CHAR_BIT));
  public:
    //store the data
    char bitmap[N/CHAR_BIT];

    //tells you if there's overflow
    template<int M>
    struct overflow_exception : public std::overflow_error {
      big_int<N> first;
      big_int<M> second;
      overflow_exception(big_int<N> first, big_int<M> second) : std::overflow_error("There was overflow.") {
	this->first = first;
	this->second = second;
      }
      
    };
    
    //default constructor - sets everything to 0
    big_int() noexcept {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
    }

    //string constructor
    big_int(std::string value, int base = 10) {
      //TODO figure this out
    }

    //copy constructor
    big_int(const big_int<N> &other) noexcept {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = other.bitmap[i];
      }
    }

    //move constructor
    big_int(big_int<N> &&other) noexcept {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = other.bitmap[i];
	other.bitmap[i] = 0;
      }
    }

    //destructor
    ~big_int() {
      //do nothing, no pointers to free/delete
    }

    //simple utility to get the number of bits
    int num_bits() noexcept {
      return N;
    }

    //copy assignment
    big_int<N> &operator=(const big_int<N> &other) noexcept {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = other.bitmap[i];
      }
      return *this;
    }

    //beware of using this; could easily lose information!
    template<int M>
    big_int<IntUtils<M, N>::max> &operator=(const big_int<M> &other) noexcept {
      for(int i = 0; i < IntUtils<M, N>::min; i++) {
	bitmap[i] = other.bitmap[i];
      }
      return *this;
    }

    //move assignment
    big_int<N> &operator=(const big_int<N> &&other) noexcept {
      big_int<N> copy(other);
      return copy;
    }

    //define other operators here: +, -, *, /, |, &, ||, &&, ==, =, etc

    //bitwise not
    big_int<N> operator~() noexcept {
      big_int<N> ret(*this);
      for(int i = 0; i < N/CHAR_BIT; i++) {
	ret.bitmap[i] = ~(ret.bitmap[i]);
      }
      return ret;
    }
    
    //negation of big_int
    big_int<N> operator-() {
      big_int<N> ret = ~(*this);
      ret += big_int<N>("1");
      return ret;
    }

    //addition of two big_ints, not necessarily of the same size
    template<int M>
    big_int<IntUtils<M, N>::max> operator+(const big_int<M> &other) {
      big_int<IntUtils<M, N>::max> ret;
      big_int<IntUtils<M, N>::min> *smaller;
      if(M >= N) {
	ret = *this;
	smaller = (big_int<IntUtils<M, N>::min> *)(&other);
      }
      else {
	ret = other;
	smaller = (big_int<IntUtils<M, N>::min> *)this;
      }
      char carry = 0;
      for(int i = 0; i < IntUtils<M, N>::min/CHAR_BIT; i++) {
	unsigned char sum = (unsigned char) ret.bitmap[i] + smaller->bitmap[i] + carry;
	carry = (sum < ret.bitmap[i]) || (sum < smaller->bitmap[i]);
	ret.bitmap[i] = sum;
      }
      int index = IntUtils<M, N>::min/CHAR_BIT;
      while(carry && (index < IntUtils<M, N>::max/CHAR_BIT)) {
	unsigned char sum = (unsigned char) ret.bitmap[index] + smaller->bitmap[index] + carry;
	carry = (sum < ret.bitmap[index]) || (sum < smaller->bitmap[index]);
	ret.bitmap[index] = sum;
	index++;
      }
      if(carry) {
	throw overflow_exception<M>(*this, other);
      }
      return ret;
    }
  };
}

#endif
