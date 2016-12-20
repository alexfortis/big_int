FLAGS=-g -Wall -Wextra -pedantic -Wfatal-errors
FILES=big_int.hpp big_int_test.cpp

big_int_test: $(FILES)
	g++ $(FLAGS) -o big_int_test $(FILES)

clean:
	rm -f big_int_test
