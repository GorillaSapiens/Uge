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
	$(CPP) -c $(CFLAGS) $< -o $@

%.o: %.cpp %.hpp
	$(CPP) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o ztest qtest

.PHONY: depend
depend:
	makedepend *.cpp

# DO NOT DELETE

qtest.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/stdint.h
qtest.o: /usr/include/assert.h /usr/include/features.h
qtest.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
qtest.o: /usr/include/printf.h /usr/include/string.h /usr/include/strings.h
qtest.o: uge_ramprintf.hpp gcstr.hpp uge_q.hpp uge_z.hpp
uge_q.o: /usr/include/math.h /usr/include/string.h /usr/include/strings.h
uge_q.o: /usr/include/features.h /usr/include/features-time64.h
uge_q.o: /usr/include/stdc-predef.h /usr/include/assert.h uge_err.hpp
uge_q.o: uge_ramprintf.hpp gcstr.hpp /usr/include/stdlib.h uge_q.hpp
uge_q.o: /usr/include/stdint.h uge_z.hpp
uge_ramprintf.o: /usr/include/stdio.h /usr/include/stdlib.h
uge_ramprintf.o: /usr/include/string.h /usr/include/strings.h
uge_ramprintf.o: /usr/include/features.h /usr/include/features-time64.h
uge_ramprintf.o: /usr/include/stdc-predef.h uge_ramprintf.hpp
uge_z.o: /usr/include/math.h /usr/include/string.h /usr/include/strings.h
uge_z.o: /usr/include/features.h /usr/include/features-time64.h
uge_z.o: /usr/include/stdc-predef.h /usr/include/assert.h uge_err.hpp
uge_z.o: uge_ramprintf.hpp gcstr.hpp /usr/include/stdlib.h uge_z.hpp
uge_z.o: /usr/include/stdint.h
ztest.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/stdint.h
ztest.o: /usr/include/assert.h /usr/include/features.h
ztest.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
ztest.o: /usr/include/printf.h /usr/include/string.h /usr/include/strings.h
ztest.o: gcstr.hpp uge_ramprintf.hpp uge_z.hpp
