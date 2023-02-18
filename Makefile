all: rational

rational: maths.cpp
	g++ -g maths.cpp -o rational

.PHONY: clean
clean:
	rm rational
