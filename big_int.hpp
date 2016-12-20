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
    static constexpr bool min = less ? A : B;
    static constexpr bool max = greater ? A : B;
  };

  template<int N>
  class big_int {
    static_assert(N % CHAR_BIT == 0, "Invalid number of bits; " STRINGIFY(N) " is not a multiple of " STRINGIFY(CHAR_BIT));
    char bitmap[N/CHAR_BIT];
  public:

    //static big_int<N> ONE("1");
    static big_int<N> ZERO;
    
    //default constructor - sets everything to 0
    big_int() {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
    }

    //string constructor
    big_int(std::string value, int base = 10) {
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

    //simple utility to get the number of bits
    int num_bits() {
      return N;
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

    //define other operators here: +, -, *, /, |, &, ||, &&, ==, =, etc

    //bitwise not
    big_int<N> operator~() {
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
    template<int A, int B>
    friend big_int<IntUtils<A, B>::max> operator+(const big_int<A> &first, const big_int<B> &second) {
      big_int<IntUtils<A, B>::max> ret;
      if(A>B) {
	ret = first;
	char carry = 0;
	for(int i = 0; i < B/CHAR_BIT; i++) {
	  char sum = ret.bitmap[i] + second.bitmap[i] + carry;
	  carry = (sum < ret.bitmap[i]) || (sum < second.bitmap[i]);
	  ret.bitmap[i] = sum;
	}
	for(int i = B/CHAR_BIT; carry && (i < A/CHAR_BIT); i++) {
	  ret.bitmap[i]++;
	  carry = !(ret.bitmap[i]);
	}
      }
      else {
	ret = second;
	char carry = 0;
	for(int i = 0; i < A/CHAR_BIT; i++) {
	  char sum = first.bitmap[i] + ret.bitmap[i] + carry;
	  carry = (sum < first.bitmap[i]) || (sum < ret.bitmap[i]);
	  ret.bitmap[i] = sum;
	}
	for(int i = A/CHAR_BIT; carry && (i < B/CHAR_BIT); i++) {
	  ret.bitmap[i]++;
	  carry = !(ret.bitmap[i]);
	}
      }
      return ret;
    }

    //subtraction
    template<int A, int B>
    friend big_int<IntUtils<A,B>::max> operator-(const big_int<A> &first, const big_int<B> &second) {
      return first + (-second);
    }
  };
}

#endif
