#include <iostream>
#include <climits>
#include <cassert>

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
    static constexpr int prod_bits = A+B;
    static constexpr int quot_bits = max-min;
    static constexpr int sum_bits = max+CHAR_BIT;
  };

  class int_t {
  protected:
    static constexpr char uppercase_digits[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static constexpr char lowercase_digits[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
  };

  template<int N>
  class big_int : int_t {
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

    std::string convert_to_base(int value, int base) {
      std::string ret;
      int cpy = value;
      while(cpy > 0) {
	std::string curr_digit(1, uppercase_digits[cpy%base]);
	ret.insert(0, curr_digit);
	cpy /= base;
      }
      return ret;
    }

    struct div_by_byte_data {
      int base;
      std::string quotient;
      char remainder;
      div_by_byte_data(int b, std::string q, char r) {
	base = b;
	quotient = q;
	remainder = r;
      }
    };
    
    //TODO: long division by 1 << CHAR_BIT
    //the value being divided should always be positive
    div_by_byte_data divide_by_byte(std::string value, int base) {
      assert(false);
      std::string ret;
      std::string curr_val;
      char rem;
      int byte_val = 1 << CHAR_BIT;
      for(int i = 0; i < value.length(); i++) {
	curr_val.push_back(value[i]);
	int curr_int_val = std::stoi(curr_val, NULL, base);
	if(curr_int_val / byte_val > 0) {
	  int curr_quot = curr_int_val / byte_val;
	  int curr_rem = curr_int_val % byte_val;
	  rem = (char)curr_rem;
	  curr_val = convert_to_base(curr_rem, base);
	  ret.push_back(convert_to_base(curr_quot, base));
	}
      }
      div_by_byte_data dbbd(base, ret, rem);
      return dbbd;
    }

    void parse(std::string value, int base) {
      int sign = 0;
      if((value[0] == '+') || (value[0] == '-')) sign = 1;
      std::string remaining = value.substr(sign);
      int counter = 1;
      while(remaining.length() > 0 && remaining != "0") {
        div_by_byte_data dbbd = divide_by_byte(remaining, base);
	bitmap[N-counter] = dbbd.remainder;
	remaining = dbbd.quotient;
	counter++;
      }
      if(sign && (value[0] == '-')) {
	*this = -(*this);
      }
    }
    
  public:
    //store the data
    unsigned char bitmap[N/CHAR_BIT];

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
    big_int() {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
    }

    //int constructor
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
      for(int i = IntUtils<M, N>::min; i < N; i++) {
	bitmap[i] = 0;
      }
      return *this;
    }

    //move assignment
    big_int<N> &operator=(const big_int<N> &&other) noexcept {
      big_int<N> copy(other);
      return copy;
    }

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
    big_int<IntUtils<M, N>::sum_bits> operator+(const big_int<M> &other) {
      big_int<IntUtils<M, N>::sum_bits> ret;
      int carry = 0;
      for(int i = 1; i <= IntUtils<M, N>::min; i++) {
	ret.bitmap[IntUtils<M, N>::sum_bits - i] = bitmap[N-i] + other.bitmap[M-i] + carry;
	carry = (ret.bitmap[IntUtils<M, N>::sum_bits - i] < bitmap[N - i]) || (ret.bitmap[IntUtils<M, N>::sum_bits - i] < other.bitmap[M - i]);
      }
      for(int index = IntUtils<M, N>::min + 1; index <= IntUtils<M, N>::max; index++) {
	if(M>N) {
	  ret.bitmap[M-index] = carry + other.bitmap[M-index];
	  carry = (ret.bitmap[M-index] < other.bitmap[M-index]);
	}
	else { //M<N because this loop won't execute if M==N
	  ret.bitmap[N-index] = carry + bitmap[N-index];
	  carry = (ret.bitmap[M-index] < bitmap[N-index]);
	}
      }
      if(carry > 0) ret.bitmap[0]++;
      return ret;
    }

    template<int M>
    big_int<N> operator+=(const big_int<M> &other) {
      //utilize the templated operator=
      *this = *this + other;
      return *this;
    }

    template<int M>
    big_int<IntUtils<M, N>::sum_bits> operator-(const big_int<M> &other) {
      return *this + (-other);
    }

    template<int M>
    big_int<N> &operator-=(const big_int<M> &other) {
      //utilize the templated operator=
      *this = *this - other;
      return *this;
    }
    
    /*template<int M>
    friend big_int<IntUtils<M, sizeof(int)*CHAR_BIT>::sum_bits> operator+(const big_int<M> &first, const int &second) {
      big_int<sizeof(int)*CHAR_BIT> ret(second);
      return first + ret;
      }

    template<int M>
    friend big_int<IntUtils<M, sizeof(int)*CHAR_BIT>::sum_bits> operator+(const int &first, const big_int<M> &second) {
      big_int<sizeof(int)*CHAR_BIT> ret(first);
      return ret + second;
      }*/

    //get string representation in any base

    //prefix operator++

    //postfix operator++

    //prefix operator--

    //postfix operator--

    //multiplication

    //division

    //bitwise and

    //bitwise or

    //bitwise xor

    //logical shift right

    //arithmetic shift right

    //shift left

    
    
  };
}

#endif
