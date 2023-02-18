all: rational

rational: maths.cpp
	g++ -g maths.cpp -o rational

powers.h: powers.pl
	./powers.pl > powers.h

.PHONY: clean
clean:
	rm rational powers.h
