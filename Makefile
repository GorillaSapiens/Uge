all: rm64 rm128

rm64: maths.cpp
	g++ maths.cpp -o rm64

rm128: maths.cpp powers.h
	g++ -DRM128 maths.cpp -o rm128

powers.h: powers.pl
	./powers.pl > powers.h

.PHONY: clean
clean:
	rm rm64 rm128
