all: rational integer uge

rational: rational_main.cpp rational.cpp rational.hpp
	g++ -g rational_main.cpp rational.cpp -o rational

integer: integer_main.cpp integer.cpp integer.hpp
	g++ -g integer_main.cpp integer.cpp -o integer

uge: uge_main.cpp uge.hpp uge.cpp integer.cpp integer.hpp
	g++ -g uge_main.cpp uge.cpp integer.cpp -o uge

.PHONY: clean
clean:
	rm -f rational integer uge
