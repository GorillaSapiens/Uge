all: rational integer

rational: rational_main.cpp rational.cpp rational.hpp
	g++ -g rational_main.cpp rational.cpp -o rational

integer: integer_main.cpp integer.cpp integer.hpp
	g++ -g integer_main.cpp integer.cpp -o integer

.PHONY: clean
clean:
	rm -f rational integer
