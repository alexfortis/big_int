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
    
    int bits_needed(long long num) {
      int ret = 0;
      while(num) {
	num >>= CHAR_BIT;
	ret++;
      }
      if(ret == 0) return CHAR_BIT;
      return ret*CHAR_BIT;
    }

    static constexpr int byte_mask = (1 << CHAR_BIT) - 1;

    //TODO: mod by 1 << CHAR_BIT
    char least_significant_byte(std::string value, int base) {
      char ret = 0;
      int counter = 0;
      int whichChar = value.length()-1;
      while(counter < CHAR_BIT) {
	int digit = std::stoi(value.substr(whichChar, 1), NULL, base);
	counter++;
      }
      return ret;
    }
    
    //TODO: long division by 1 << CHAR_BIT
    std::string divide_by_byte(std::string value, int base) {
      std::string ret;
      
      return ret;
    }

    void parse(std::string value, int base) {
      std::string remaining(value);
      int counter = 0;
      while(remaining.length() > 0) {
	bitmap[counter] = least_significant_byte(value, base);
	remaining = divide_by_byte(remaining, base);
	counter++;
      }
    }
    
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

    //int constructor
    std::enable_if<!IntUtils<N/CHAR_BIT, sizeof(int)>::less>
    big_int(int value) {
      for(int i = 0; i < N; i++) {
        if(i < sizeof(int)) {
	  bitmap[i] = ((byte_mask << (i*CHAR_BIT)) & value) >> (i*CHAR_BIT);
	}
	else {
	  bitmap[i] = 0;
	}
      }
    }

    //string constructor
    big_int(std::string value, int base = 10) {
      //initialize everything first
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
      //now parse the string
      parse(value, base);
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
    big_int &operator=(const big_int &other) noexcept {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = other.bitmap[i];
      }
      return *this;
    }

    //beware of using this; could easily lose information!
    template<int M>
    big_int<N> &operator=(const big_int<M> &other) noexcept {
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
