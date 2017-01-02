#include <iostream>
#include <climits>
#include <cassert>

#ifndef BIG_INT_H
#define BIG_INT_H

#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define STRINGIFY_HELPER(x) #x

#define DEBUG 1

namespace alexstrong {

  static constexpr char uppercase_digits[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr char lowercase_digits[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
  
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

  template<int N>
  class big_int {
    static_assert(N % CHAR_BIT == 0, "Invalid number of bits; " STRINGIFY(N) " is not a multiple of " STRINGIFY(CHAR_BIT));
    static_assert(N>0, "Number of bits must be positive.");
    
    int bits_needed(long long num) {
      int ret = 0;
      while(num) {
	num >>= CHAR_BIT;
	ret++;
      }
      if(ret == 0) return CHAR_BIT;
      return ret*CHAR_BIT;
    }

    template<int M>
    struct division_data {
      big_int<N> quotient;
      big_int<M> remainder;
      bool error;
      division_data() : quotient(0), remainder(0), error(false) {
      }
    };

    template<int M>
    division_data<M> divide(const big_int<M> &other) const noexcept {
      division_data<M> ret;
      //make sure you don't divide by 0!
      if(other.sign() && (-other).sign()) {
	ret.error = true;
	return ret;
      }
      //now that we know other isn't 0, move on.
      big_int<IntUtils<M, N>::max> abs_this(this->abs());
      big_int<IntUtils<M, N>::max> abs_other(other.abs());
      bool neg = sign() ^ other.sign();
      ret.remainder = *this;
      if(abs_this < abs_other) {
	//quotient should already be 0, but just in case...
	ret.quotient = big_int<M>(0);
	//make sure the remainder is between 0 and abs_other, inclusive
	while(!ret.remainder.sign()) {
	  ret.remainder += abs_other;
	  ret.quotient--;
	}
      }
      else {
	int msb_this = abs_this.msb();
	int msb_other = abs_other.msb();
	int diff = msb_this - msb_other;
	//if the difference is 0 or 1, we can just do it without too much fuss
	if(diff <= 1) {
	  while(ret.remainder >= abs_other) {
	    ret.remainder -= abs_other;
	    ret.quotient++;
	  }
	  while(!ret.remainder.sign()) {
	    ret.remainder += abs_other;
	    ret.quotient--;
	  }
	}
	//otherwise, we need to do some shifting
	else {
	  //we know N > M here
	  big_int<N> shifted = big_int<N>(abs_other) << ((diff-1)*CHAR_BIT);
	  big_int<N> quot(1);
	  quot <<= ((diff-1)*CHAR_BIT);
	  ret.remainder = abs_this;
	  int numOff = 0;
	  while(ret.remainder >= abs_other) {
	    while(ret.remainder >= shifted) {
	      ret.remainder -= shifted;
	      ret.quotient += quot;
	    }
	    while(!ret.remainder.sign()) {
	      ret.remainder += shifted;
	      ret.quotient -= quot;
	    }
	    numOff++;
	    shifted = abs_other << ((diff-1-numOff)*CHAR_BIT);
	    quot >>= CHAR_BIT;
	  }
	}
      }
      if(neg) {
	ret.quotient = -(ret.quotient+1);
	ret.remainder = abs_other - ret.remainder;
      }
      return ret;
    }

    static const int byte_mask = (1 << CHAR_BIT) - 1;

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
    div_by_byte_data divide_by_byte(const std::string &value, const int &base) {
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
	  std::string quot_str = convert_to_base(curr_quot, base);
	  ret.insert(ret.length(), quot_str);
	}
	else {
	  rem = (char)(curr_int_val % byte_val);
	  ret.push_back('0');
	}
      }
      int first_nonzero = 0;
      while(first_nonzero < (int)ret.length() && ret.at(first_nonzero) == '0') first_nonzero++;
      if(first_nonzero >= (int)(ret.length())) {
	ret = "";
      }
      else {
	ret = ret.substr(first_nonzero);
      }
      div_by_byte_data dbbd(base, ret, rem);
      return dbbd;
    }

    void parse(const std::string &value, int base) {
      if(value.length() == 0) {
	for(int i = 0; i < N/CHAR_BIT; i++) {
	  bitmap[i] = 0;
	}
	return;
      }
      int is_sign = 0;
      if((value[0] == '+') || (value[0] == '-')) is_sign = 1;
      if((int)value.length() == is_sign) {
	for(int i = 0; i < N/CHAR_BIT; i++) {
	  bitmap[i] = 0;
	}
	return;
      }
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

    //get the most significant byte that is not equal to 0
    //return -1 if the number is equal to 0
    int msb() {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	if(bitmap[i] != 0) {
	  return N/CHAR_BIT-i-1;
	}
      }
      return -1;
    }
    
    //store the data
    unsigned char bitmap[N/CHAR_BIT];
    
  public:
    template<int M>
    friend class alexstrong::big_int;
    
    //static stuff, stores info about the size
    static constexpr int num_bits = N;
    static constexpr int num_bytes = N/CHAR_BIT;
    
    //default constructor - sets everything to 0
    big_int() {
      for(int i = 0; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
    }

    //int constructor
    big_int(int value) {
      for(int i = 0; i < N/CHAR_BIT; i++) {
        if(i < (int)sizeof(int)) {
	  bitmap[N/CHAR_BIT - 1 - i] = ((byte_mask << (i*CHAR_BIT)) & value) >> (i*CHAR_BIT);
	}
	else {
	  bitmap[N/CHAR_BIT - 1 - i] = 0;
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
      //if(DEBUG) std::cerr << "other's int value is " << other.to_int() << std::endl;
      if(DEBUG) std::cerr << "There are " << N/CHAR_BIT << " bytes in this number." << std::endl;
      for(int i = 0; i < N/CHAR_BIT; i++) {
	if(DEBUG) std::cerr << "\tcopy constructor loop: i = " << i << std::endl;
	bitmap[i] = other.bitmap[i];
      }
    }

    //beware of using this; could easily use information!
    template<int M>
    big_int(const big_int<M> &other) noexcept {
      if(DEBUG) std::cerr << "Copying a " << M << "-bit number into a " << N << "-bit number." << std::endl;
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
	if(DEBUG) std::cerr << "\tsetting index " << N/CHAR_BIT-i << std::endl;
	bitmap[N/CHAR_BIT-i] = other.bitmap[M/CHAR_BIT-i];
      }
      if(DEBUG) std::cerr << "\tcopied all the bits" << std::endl;
      for(int i = IntUtils<M, N>::min/CHAR_BIT+1; i <= N/CHAR_BIT; i++) {
	if(DEBUG) std::cerr << "\tsetting index " << N/CHAR_BIT-i << std::endl;
	bitmap[N/CHAR_BIT-i] = (!(other.sign())) * byte_mask;
      }
      //ensure the sign of this number is the same as the other one
      if(N<M) {
	if(other.sign() != sign()) {
	  bitmap[0] ^= (1 << (CHAR_BIT-1));
	}
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
      //if(DEBUG) std::cerr << to_int();
      big_int<N> ret;
      for(int i = 0; i < N/CHAR_BIT; i++) {
	char notted = ~(bitmap[i]);
	ret.bitmap[i] = notted;
      }
      return ret;
    }
    
    //negation of big_int
    big_int<N> operator-() const noexcept {
      big_int<N> ret(~(*this));
      big_int<N> ONE(1);
      ret += ONE;
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
    big_int<N> &operator+=(const big_int<M> &other) noexcept {
      //create a copy of the other one that's the same size as *this
      //so two's-complement arithmetic can actually work.
      big_int<N> copy(other);
      bool carry = 0;
      for(int i = 1; i <= N/CHAR_BIT; i++) {
	int index = N/CHAR_BIT-i;
	unsigned char sum = (unsigned char)carry + bitmap[index] + copy.bitmap[index];
	carry = (sum < bitmap[index]) || (sum < copy.bitmap[index]);
	bitmap[index] = sum;
      }
      return *this;
    }

    big_int<N> &operator+=(const int &a) {
      big_int<sizeof(int)*CHAR_BIT> a_as_big_int(a);
      return *this += a_as_big_int;
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
      big_int<IntUtils<M, N>::sum_bits> diff(*this);
      diff -= other;
      return diff;
    }

    template<int M>
    big_int<N> &operator-=(const big_int<M> &other) noexcept {
      big_int<M> neg(-other);
      assert(neg == -other);
      assert(-neg == other);
      *this += neg;
      return *this;
    }

    big_int<N> &operator-=(const int &other) noexcept {
      big_int<CHAR_BIT * sizeof(int)> neg(-other);
      *this += neg;
      return *this;
    }

    //subtracting int/big_int
    friend big_int<IntUtils<CHAR_BIT*sizeof(int), N>::sum_bits> operator-(const big_int<N> &a, const int &b) noexcept {
      big_int<IntUtils<CHAR_BIT*sizeof(int), N>::sum_bits> ret(a);
      ret -= b;
      return ret;
    }

    friend big_int<IntUtils<CHAR_BIT*sizeof(int), N>::sum_bits> operator-(const int &a, const big_int<N> &b) noexcept {
      big_int<IntUtils<CHAR_BIT*sizeof(int), N>::sum_bits> ret(b);
      ret -= a;
      return ret;
    }

    //returns true if positive, false if negative
    bool sign() const{
      int mask = 1 << (CHAR_BIT-1);
      return !(bitmap[0] & mask);
    }

    big_int<N> abs() const noexcept {
      if(sign()) return *this;
      return -(*this);
    }

    template<int M>
    bool operator<(const big_int<M> &other) const noexcept {
      if(sign() && !(other.sign())) return false;
      if(other.sign() && !sign()) return true;
      //now we know they both have the same sign
      big_int<N> abs_this = this->abs();
      big_int<M> abs_other = other.abs();
      bool ret = false;
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
	if(abs_this.bitmap[N/CHAR_BIT-i] < abs_other.bitmap[M/CHAR_BIT-i]) {
	  ret = sign();
	}
	else if(abs_this.bitmap[N/CHAR_BIT-i] > abs_other.bitmap[M/CHAR_BIT-i]) {
	  ret = !sign();
	}
      }
      if(M>N) {
	for(int i = N/CHAR_BIT + 1; i <= M/CHAR_BIT; i++) {
	  if(abs_other.bitmap[M/CHAR_BIT-i] > 0) {
	    return other.sign();
	  }
	}
      }
      else if(N>M) {
	for(int i = M/CHAR_BIT + 1; i <= N/CHAR_BIT; i++) {
	  if(abs_this.bitmap[N/CHAR_BIT-i] > 0) {
	    return !sign();
	  }
	}
      }
      return ret;
    }

    template<int M>
    bool operator==(const big_int<M> &other) const noexcept {
      return !((*this < other) || (other < *this));
    }

    template<int M>
    bool operator!=(const big_int<M> &other) const noexcept {
      return !(*this == other);
    }

    template<int M>
    bool operator>(const big_int<M> &other) const noexcept {
      return other < *this;
    }

    template<int M>
    bool operator<=(const big_int<M> &other) const noexcept {
      return !(*this > other);
    }

    template<int M>
    bool operator>=(const big_int<M> &other) const noexcept {
      return !(*this < other);
    }

    //division operator
    template<int M>
    big_int<N> operator/(const big_int<M> &other) const noexcept {
      /*big_int<N> mod(*this);
      big_int<M> limit(other);
      big_int<N> ret; //equals 0
      big_int<N> ONE(1);
      //ensure both numbers are positive
      if(!(other.sign())) {
	limit = -limit;
      }
      if(!(this->sign())) {
	mod = -mod;
      }
      if(DEBUG) std::cerr << "Is mod greater than or equal to limit? " << (mod>=limit) << std::endl;
      while(mod >= limit) {
	mod -= limit;
	ret += ONE;
      }
      if(other.sign() ^ sign()) {
	ret = -ret;
      }
      return ret;*/
      division_data<M> data = divide(other);
      return data.quotient;
    }

    template<int M>
    big_int<IntUtils<M, N>::min> operator%(const big_int<M> &other) const noexcept {
      /*big_int<N> mod(*this);
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
      if((other.sign()) ^ (mod.sign())) {
	ret = -ret;
      }
      return big_int<IntUtils<M, N>::min>(mod);*/
      division_data<M> data = divide(other);
      big_int<IntUtils<M, N>::min> ret(data.remainder);
      return ret;
    }

    big_int<N> operator/(const int &other) const noexcept {
      big_int<CHAR_BIT * sizeof(int)> other_as_big_int(other);
      return *this / other_as_big_int;
    }

    int to_int() const {
      int ret = 0;
      big_int<N> copy((this->sign())?(*this):(-(*this)));
      for(int i = 0; (i < (int)sizeof(int)) && (i < N/CHAR_BIT); i++) {
	if(DEBUG) std::cerr << N << "/" << CHAR_BIT << " is " << N/CHAR_BIT  << std::endl;
	if(DEBUG) std::cerr << "i is " << i << std::endl;
	if(DEBUG) std::cerr << "ret: " << ret << " --> ";
	int this_byte = (int)(copy.bitmap[N/CHAR_BIT - i - 1]);
	ret += (this_byte << (i*CHAR_BIT));
	if(DEBUG) std::cerr << ret << std::endl;
      }
      if(!sign()) {
	ret = -ret;
      }
      return ret;
    }

    int operator%(const int &other) const {
      big_int<sizeof(int)*CHAR_BIT> other_as_big_int(other);
      return (*this % other_as_big_int).to_int();
    }

    long to_long() const {
      long ret = 0;
      for(int i = 0; i < sizeof(long) && i < N/CHAR_BIT; i++) {
	ret += (long)bitmap[N/CHAR_BIT-i-1] << (i*CHAR_BIT);
      }
      return ret;
    }

    long long to_long_long() const {
      long long ret = 0;
      for(int i = 0; i < sizeof(long long) && i < N/CHAR_BIT; i++) {
	ret += (long long)bitmap[N/CHAR_BIT-i-1] << (i*CHAR_BIT);
      }
      return ret;
    }

    //get string representation in any base <= 36
    std::string to_base(int base) {
      assert(base <= 36 && base > 1);
      big_int<sizeof(int)*CHAR_BIT> base_big_int(base);
      std::string ret = "";
      const big_int<N> ZERO;
      big_int<N> copy(*this);
      if(!sign()) copy = -copy;
      if(DEBUG) std::cerr << "copy, as int, is " << copy.to_int() << std::endl;
      while(copy != ZERO) {
        auto div_data = copy.divide(base_big_int);
	big_int<N> quotient = div_data.quotient;
	if(DEBUG) std::cerr << "quotient, as int, is " << quotient.to_int() << std::endl;
        int remainder = div_data.remainder.to_int();
	if(DEBUG) std::cerr << "remainder is " << remainder << std::endl;
	char next_digit = uppercase_digits[remainder];
	if(DEBUG) std::cerr << "next digit is " << next_digit << std::endl;
	ret.insert(ret.begin(), next_digit);
	if(DEBUG) std::cerr << "ret is " << ret << std::endl;
	copy = quotient;
	if(DEBUG) std::cerr << "copy, as int, is " << copy.to_int() << std::endl;
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
    big_int<N> &operator++() {
      *this += 1;
      return *this;
    }

    //postfix operator++
    big_int<N> operator++(int) {
      big_int<N> copy(*this);
      ++(*this);
      return copy;
    }

    //prefix operator--
    big_int<N> &operator--() {
      *this -= 1;
      return *this;
    }

    //postfix operator--
    big_int<N> operator--(int) {
      big_int<N> copy(*this);
      ++(*this);
      return copy;
    }

    //multiplication
    //there is overflow 
    template<int M>
    big_int<N> &operator*=(const big_int<M> &other) {
      big_int<N> nzero;
      if(*this == nzero) return *this;
      big_int<M> zero;
      if(other == zero) {
	for(int i = 0; i < N/CHAR_BIT; i++) {
	  bitmap[i] = 0;
	}
      }
      else {
	//do log-time multiplication by doubling
	big_int<M> abs_other(other.abs());
	if(!(other.sign())) {
	  *this = -(*this);
	}
	big_int<M> copy(abs_other);
	big_int<N> sum; //should be 0
	while(copy-2 != -copy) {
	  //if it's odd
	  if(copy.bitmap[M/CHAR_BIT-1] & 1) {
	    sum += *this;
	    copy--;
	  }
	  *this += *this;
	  copy >>= 1;
	}
	*this += sum;
      }
      return *this;
    }

    template<int M>
    big_int<M+N> operator*(const big_int<M> &other) {
      big_int<M+N> ret;
      ret *= other;
      return ret;
    }

    //bitwise and
    template<int M>
    big_int<N> &operator&=(const big_int<M> &other) {
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
	bitmap[N/CHAR_BIT-i] &= other.bitmap[M/CHAR_BIT-i];
      }
      for(int i = IntUtils<M, N>::min/CHAR_BIT + 1; i <= N/CHAR_BIT; i++) {
	bitmap[N/CHAR_BIT-i] = 0;
      }
      return *this;
    }

    template<int M>
    big_int<IntUtils<M, N>::max> operator&(const big_int<M> &other) {
      big_int<IntUtils<N, N>::max> ret(*this);
      ret &= other;
      return ret;
    }

    //bitwise or
    template<int M>
    big_int<N> &operator|=(const big_int<M> &other) {
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
	bitmap[N/CHAR_BIT-i] |= other.bitmap[M/CHAR_BIT-i];
      }
      //for higher bytes, or it with 0, or do nothing.
      //more efficient to do nothing, so just return.
      return *this;
    }

    template<int M>
    big_int<IntUtils<M, N>::max> operator|(const big_int<M> &other) {
      big_int<IntUtils<M, N>::max> ret(*this);
      ret |= other;
      return ret;
    }

    //bitwise xor
    template<int M>
    big_int<N> &operator^=(const big_int<M> &other) {
      for(int i = 1; i <= IntUtils<M, N>::min/CHAR_BIT; i++) {
	bitmap[N/CHAR_BIT-i] ^= other.bitmap[M/CHAR_BIT-i];
      }
      //XOR the other bytes with 0, i.e. do nothing
      return *this;
    }

    template<int M>
    big_int<IntUtils<M, N>::max> operator^(const big_int<M> &other) {
      big_int<IntUtils<M, N>::max> ret(*this);
      ret ^= other;
      return ret;
    }

    //arithmetic shift right
    big_int<N> &operator>>=(const int &other) noexcept {
      const int quot = other/CHAR_BIT;
      const int rem = other%CHAR_BIT;
      const int mask = (1<<rem)-1;
      //first, shift by the remainder
      //first do the first byte, then the rest
      char prev_bits = (bitmap[0] & mask) << (CHAR_BIT-rem);
      bitmap[0] >>= rem;
      for(int i = 1; i < N/CHAR_BIT; i++) {
	char new_prev_bits = (bitmap[i] & mask) << (CHAR_BIT-rem);
	bitmap[i] >>= rem;
	bitmap[i] |= prev_bits;
	prev_bits = new_prev_bits;
      }
      //then, move the bytes by the quotient
      //new_byte should be either 0 or 2^CHAR_BIT-1 depending on the sign
      unsigned char new_byte = (!sign())*byte_mask;
      for(int i = N/CHAR_BIT-1; i >= quot; i--) {
	bitmap[i] = bitmap[i-quot];
      }
      for(int i = quot-1; i >= 0; i--) {
	bitmap[i] = new_byte;
      }
      return *this;
    }

    /*template<int M>
    big_int<N> &operator>>=(const big_int<M> &other) {
      big_int<M> copy(other);
      while(copy > big_int(0)) {
	int x = copy.to_int();
	*this >>= x;
	copy >>= (sizeof(int) * CHAR_BIT);
      }
      return *this;
      }*/

    big_int<N> operator>>(const int &other) const noexcept {
      big_int<N> ret(*this);
      ret >>= other;
      return ret;
    }

    /*template<int M>
    big_int<N> operator>>(const big_int<M> &other) {
      big_int<N> ret(*this);
      ret >>= other;
      return ret;
      }*/

    //shift left
    big_int<N> &operator<<=(const int &other) noexcept {
      const int quot = other / CHAR_BIT;
      const int rem = other % CHAR_BIT;
      const int mask = byte_mask ^ ((1 << (CHAR_BIT-rem))-1);
      //first go through each byte and move the bits to the left by rem
      //last byte first
      //set prev_bits to be the first rem bits of bitmap[N/CHAR_BIT-1]
      char prev_bits = (bitmap[N/CHAR_BIT-1] & mask) >> rem;
      //move it over by rem bits
      bitmap[N/CHAR_BIT-1] <<= rem;
      //now the other bytes
      for(int i = N/CHAR_BIT-2; i >= 0; i--) {
	char new_prev_bits = (bitmap[i] & mask) >> rem;
	bitmap[i] <<= rem;
	bitmap[i] |= prev_bits;
	prev_bits = new_prev_bits;
      }
      //now move the bytes
      for(int i = 0; i < N/CHAR_BIT-quot; i++) {
	bitmap[i] = bitmap[i+quot];
      }
      for(int i = N/CHAR_BIT-quot; i < N/CHAR_BIT; i++) {
	bitmap[i] = 0;
      }
      return *this;
    }

    /*template<int M>
    big_int<N> &operator<<=(const big_int<M> &other) {
      big_int<M> copy(other);
      while(copy > big_int(0)) {
	int copy_int = copy.to_int();
	*this >>= copy_int;
	copy >>= (sizeof(int)*CHAR_BIT);
      }
      }*/

    big_int<N> operator<<(const int &other) const noexcept {
      big_int<N> ret(*this);
      ret <<= other;
      return ret;
    }
    
  };
}

#endif
