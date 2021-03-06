FLAGS=-Wall -Wextra -pedantic -Wfatal-errors
FILES=big_int.hpp big_int_test.cpp

with_gcc: $(FILES)
	g++ -g $(FLAGS) -o big_int_test $(FILES) -std=c++14

with_clang: $(FILES)
	clang++ -g -c $(FLAGS) $(FILES) -std=c++14
	clang++ *.o -o big_int_test
	rm *.o

clean:
	rm -f big_int_test
