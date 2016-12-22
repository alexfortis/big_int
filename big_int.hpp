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
    static_assert(N>0, "Cannot have a 0-bit number. That makes no sense!");

    char int_t::uppercase_digits[37];
    char int_t::lowercase_digits[37];
    
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

    std::string convert_to_base(const int &value, const int &base) {
      std::string ret = "";
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
      div_by_byte_data(int b=10, std::string q = "1", char r = '0') : base(b), quotient(q), remainder(r) {
      }
      div_by_byte_data &operator=(const div_by_byte_data &other) {
	base = other.base;
	quotient = other.quotient;
	remainder = other.remainder;
	return *this;
      }
    };
    
    //TODO: long division by 1 << CHAR_BIT
    //the value being divided should always be positive
    div_by_byte_data divide_by_byte(const std::string &value, int base) {
      std::string ret = "";
      std::string curr_val = "";
      char rem = 0;
      int byte_val = 1 << CHAR_BIT;
      for(unsigned i = 0; i < value.length(); i++) {
	curr_val.push_back(value.at(i));
	int curr_int_val = std::stoi(curr_val, NULL, base);
	if(curr_int_val / byte_val > 0) {
	  int curr_quot = curr_int_val / byte_val;
	  int curr_rem = curr_int_val % byte_val;
	  rem = (char)curr_rem;
	  curr_val = convert_to_base(curr_rem, base);
	  ret.insert(ret.length(), convert_to_base(curr_quot, base));
	}
	else {
	  rem = (char)(curr_int_val % byte_val);
	}
      }
      div_by_byte_data dbbd(base, ret, rem);
      return dbbd;
    }

    void parse(const std::string &value, int base) {
      int is_sign = 0;
      if((value[0] == '+') || (value[0] == '-')) is_sign = 1;
      std::string remaining = value.substr(is_sign);
      int counter = 1;
      div_by_byte_data dbbd;
      while(remaining.length() > 0) {
        dbbd = divide_by_byte(remaining, base);
	bitmap[N/CHAR_BIT-counter] = dbbd.remainder;
	remaining = dbbd.quotient;
	counter++;
      }
      if(is_sign && (value[0] == '-')) {
	*this = -(*this);
      }
    }
    
  public:
    //store the data
    unsigned char bitmap[N/CHAR_BIT];
    
    //default constructor - sets everything to 0
    big_int() {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
    }

    //int constructor
    big_int(int value) {
      for(int i = 0; i < N; i++) {
        if(i < (int)sizeof(int)) {
	  bitmap[i] = ((byte_mask << (i*CHAR_BIT)) & value) >> (i*CHAR_BIT);
	}
	else {
	  bitmap[i] = 0;
	}
      }
    }

    //string constructor
    big_int(const std::string &value, int base = 10) noexcept {
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

    //beware of using this; could easily use information!
    template<int M>
    big_int(const big_int<M> &other) noexcept {
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
	bitmap[N/CHAR_BIT-i] = other.bitmap[M/CHAR_BIT-i];
      }
      //ensure the sign of this number is the same as the other one
      if(N<M) {
	if(other.sign() != sign()) {
	  bitmap[0] ^= 1 << (CHAR_BIT-1);
	}
      }
      for(int i = IntUtils<M, N>::min+1; i <= N; i++) {
	bitmap[N/CHAR_BIT-i] = (!(other.sign())) * byte_mask;
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
    //~big_int() {
      //do nothing, no pointers to free/delete
    //}

    //simple utility to get the number of bits
    constexpr int num_bits() noexcept {
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
      for(int i = 1; i <= IntUtils<M, N>::min; i++) {
	bitmap[N-i] = other.bitmap[M-i];
      }
      //ensure the sign of this number is the same as the other one
      if(N<M) {
	if(other.sign() != sign()) {
	  bitmap[0] ^= 1 << (CHAR_BIT-1);
	}
      }
      for(int i = IntUtils<M, N>::min+1; i <= N; i++) {
	bitmap[N-i] = (!(other.sign())) * byte_mask;
      }
      return *this;
    }

    //move assignment
    big_int<N> &operator=(big_int<N> &&other) noexcept {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = other.bitmap[i];
	other.bitmap[i] = 0;
      }
      return *this;
    }

    //bitwise not
    big_int<N> operator~() const noexcept {
      big_int<N> ret(*this);
      for(int i = 0; i < N/CHAR_BIT; i++) {
	ret.bitmap[i] = ~(ret.bitmap[i]);
      }
      return ret;
    }
    
    //negation of big_int
    big_int<N> operator-() const noexcept {
      big_int<N> ret = ~(*this);
      ret += big_int<N>(1);
      return ret;
    }

    //addition of two big_ints, not necessarily of the same size
    template<int M>
    big_int<IntUtils<M, N>::sum_bits> operator+(const big_int<M> &other) const noexcept {
      big_int<IntUtils<M, N>::sum_bits> ret(*this);
      ret += other;
      return ret;
    }

    //beware of overflow when using this! but if you allocate enough bits, you'll probably be fine.
    template<int M>
    big_int<N> operator+=(const big_int<M> &other) noexcept {
      int carry = 0;
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
        unsigned char new_char = bitmap[N/CHAR_BIT-i] + other.bitmap[M/CHAR_BIT-i] + (unsigned char)carry;
	carry = (new_char < bitmap[N/CHAR_BIT - i]) || (new_char < other.bitmap[M/CHAR_BIT - i]);
	bitmap[N/CHAR_BIT - i] = new_char;
      }
      for(int index = (IntUtils<M, N>::min)/CHAR_BIT + 1; index <= N/CHAR_BIT; index++) {
        unsigned char new_char = (unsigned char)carry + bitmap[N/CHAR_BIT-index];
	carry = (new_char < bitmap[N/CHAR_BIT-index]);
	bitmap[N/CHAR_BIT - index] = new_char;
      }
      if(carry > 0) {
	bitmap[0]++;
      }
      return *this;
    }
    
    //adding an int to a big_int
    friend big_int<IntUtils<N, sizeof(int)*CHAR_BIT>::sum_bits> operator+(const int &a, const big_int<N> &b) noexcept{
      big_int<sizeof(int)*CHAR_BIT> a_as_big_int(a);
      return a_as_big_int + b;
    }

    friend big_int<IntUtils<N, sizeof(int)*CHAR_BIT>::sum_bits> operator+(const big_int<N> &a, const int &b) noexcept {
      return b + a;
    }

    //subtraction of two big_ints, not necessarily of the same size
    template<int M>
    big_int<IntUtils<M, N>::sum_bits> operator-(const big_int<M> &other) const noexcept {
      return *this + (-other);
    }

    template<int M>
    big_int<N> &operator-=(const big_int<M> &other) noexcept {
      //utilize the templated operator=
      *this = *this - other;
      return *this;
    }

    //returns true if positive, false if negative
    bool sign() const{
      int mask = 1 << (CHAR_BIT-1);
      return !(bitmap[0] & mask);
    }

    template<int M>
    bool operator<(const big_int<M> &other) noexcept {
      big_int<IntUtils<M, N>::sum_bits> diff = other - *this;
      big_int<IntUtils<M, N>::sum_bits> diff2 = *this - other;
      if(diff == diff2) return false; //this means they're equal
      return diff.sign();
    }

    template<int M>
    bool operator==(const big_int<M> &other) noexcept {
      std::cout << "comparing a big_int<" << N << "> with a big_int<" << M << ">" <<std::endl;
      int thisIndex = 0, otherIndex = 0;
      std::cout << "thisIndex: " << thisIndex << std::endl;
      std::cout << "otherIndex: " << otherIndex << std::endl;
      if(M>N) {
	for(otherIndex = 0; otherIndex < (M-N)/CHAR_BIT; otherIndex++) {
	  if(other.bitmap[otherIndex]) return false;
	}
      }
      else if(N>M) {
	for(thisIndex = 0; thisIndex < (N-M)/CHAR_BIT; thisIndex++) {
	  if(this->bitmap[thisIndex]) return false;
	}
      }
      std::cout << "thisIndex: " << thisIndex << std::endl;
      std::cout << "otherIndex: " << otherIndex << std::endl;
      for(; thisIndex < N/CHAR_BIT && otherIndex < M/CHAR_BIT; thisIndex++, otherIndex++) {
	std::cout << "thisIndex: " << thisIndex << std::endl;
	std::cout << "otherIndex: " << otherIndex << std::endl;
	char thisByte = bitmap[thisIndex];
	char otherByte = other.bitmap[otherIndex];
	std::cout << "Comparing equality of " << (int)thisByte << " and " << (int)otherByte << std::endl;
	if(thisByte != otherByte) {
	  std::cout << "returning false" << std::endl;
	  return false;
	}
      }
      std::cout << "returning true" << std::endl;
      return true;
    }

    template<int M>
    bool operator!=(const big_int<M> &other) noexcept {
      return !(*this == other);
    }

    template<int M>
    bool operator>(const big_int<M> &other) noexcept {
      return other < *this;
    }

    template<int M>
    bool operator<=(const big_int<M> &other) noexcept {
      return !(*this > other);
    }

    template<int M>
    bool operator>=(const big_int<M> &other) noexcept {
      return !(*this < other);
    }

    //division operator
    template<int M>
    big_int<N> operator/(const big_int<M> &other) noexcept {
      big_int<N> mod(*this);
      big_int<M> limit(other);
      big_int<N> ret; //equals 0
      big_int<N> ONE(1);
      //ensure both numbers are positive
      if(!(other.sign())) {
	limit = -limit;
      }
      if(!(mod.sign())) {
	mod = -mod;
      }
      while(mod >= limit) {
	mod -= limit;
	ret += ONE;
      }
      if(other.sign() ^ sign()) {
	ret = -ret;
      }
      return ret;
    }

    template<int M>
    big_int<IntUtils<M, N>::min> operator%(const big_int<M> &other) noexcept {
      big_int<N> mod(*this);
      big_int<M> limit(other);
      big_int<N> ret; //equals 0
      big_int<N> ONE("1");
      //ensure both numbers are positive
      if(!(other.sign())) {
	limit = -limit;
      }
      if(!(mod.sign())) {
	mod = -mod;
      }
      while(mod >= limit) {
	mod -= limit;
	ret += ONE;
      }
      if((other.sign()) ^ (mod.sign())) {
	ret = -ret;
      }
      return big_int<IntUtils<M, N>::min>(mod);
    }

    big_int<N> operator/(const int &other) noexcept {
      big_int<CHAR_BIT * sizeof(int)> other_as_big_int(other);
      return *this / other_as_big_int;
    }

    int to_int() {
      int ret = 0;
      for(int i = 0; i < (int)sizeof(int) && i < N; i++) {
	ret += (int)bitmap[N-i-1] << (i*CHAR_BIT);
      }
      return ret;
    }

    int operator%(const int &other) {
      big_int<sizeof(int)*CHAR_BIT> other_as_big_int(other);
      return (*this % other_as_big_int).to_int();
    }

    long to_long() {
      long ret = 0;
      for(int i = 0; i < sizeof(long) && i < N; i++) {
	ret += (long)bitmap[N-i-1] << (i*CHAR_BIT);
      }
      return ret;
    }

    long long to_long_long() {
      long long ret = 0;
      for(int i = 0; i < sizeof(long long) && i < N; i++) {
	ret += (long long)bitmap[N-i-1] << (i*CHAR_BIT);
      }
      return ret;
    }

    //get string representation in any base <= 36
    std::string to_base(int base) {
      assert(base <= 36 && base > 1);
      std::string ret = "";
      const big_int<N> ZERO;
      big_int<N> copy(*this);
      if(!sign()) copy = -copy;
      while(copy != ZERO) {
	big_int<N> quotient = copy/base;
        int remainder = copy % base;
	ret.insert(ret.begin(), uppercase_digits[remainder]);
	copy = quotient;
      }
      if(!sign()) ret.insert(0, "-");
      return ret;
    }

    //output
    friend std::ostream &operator<<(std::ostream &os, big_int<N> num) {
      os << num.to_base(10);
      return os;
    }

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
