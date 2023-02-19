all: rational

rational: main.cpp rational.cpp rational.h
	g++ -g main.cpp rational.cpp -o rational

.PHONY: clean
clean:
	rm rational
