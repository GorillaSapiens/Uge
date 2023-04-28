CPP=g++
CFLAGS=-g

OBJS=\
   uge_z.o \
   uge_q.o \
   uge_ramprintf.o

all: ztest qtest

ztest: ztest.o $(OBJS)
	$(CPP) $(CFLAGS) ztest.o $(OBJS) -o ztest

qtest: qtest.o $(OBJS)
	$(CPP) $(CFLAGS) qtest.o $(OBJS) -o qtest

%.o: %.cpp
	$(CPP) -c $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f *.o ztest qtest

# DO NOT DELETE
