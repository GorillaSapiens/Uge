all: rational

rational: main.cpp rational.cpp rational.hpp
	g++ -g main.cpp rational.cpp -o rational

.PHONY: clean
clean:
	rm rational
