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

qtest.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
qtest.o: /usr/include/_newlib_version.h /usr/include/sys/config.h
qtest.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
qtest.o: /usr/include/sys/cdefs.h /usr/include/machine/_default_types.h
qtest.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
qtest.o: /usr/include/machine/_types.h /usr/include/sys/lock.h
qtest.o: /usr/include/sys/stdio.h /usr/include/stdlib.h
qtest.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
qtest.o: /usr/include/stdint.h /usr/include/sys/_intsup.h
qtest.o: /usr/include/sys/_stdint.h /usr/include/assert.h
qtest.o: /usr/include/string.h /usr/include/sys/_locale.h
qtest.o: /usr/include/strings.h /usr/include/sys/string.h uge_ramprintf.hpp
qtest.o: gcstr.hpp uge_q.hpp uge_z.hpp
uge_q.o: /usr/include/math.h /usr/include/sys/reent.h /usr/include/_ansi.h
uge_q.o: /usr/include/newlib.h /usr/include/_newlib_version.h
uge_q.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
uge_q.o: /usr/include/sys/features.h /usr/include/sys/cdefs.h
uge_q.o: /usr/include/machine/_default_types.h /usr/include/sys/_types.h
uge_q.o: /usr/include/machine/_types.h /usr/include/sys/lock.h
uge_q.o: /usr/include/string.h /usr/include/sys/_locale.h
uge_q.o: /usr/include/strings.h /usr/include/sys/string.h
uge_q.o: /usr/include/assert.h uge_err.hpp uge_ramprintf.hpp gcstr.hpp
uge_q.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
uge_q.o: /usr/include/alloca.h uge_q.hpp /usr/include/stdint.h
uge_q.o: /usr/include/sys/_intsup.h /usr/include/sys/_stdint.h uge_z.hpp
uge_ramprintf.o: /usr/include/stdio.h /usr/include/_ansi.h
uge_ramprintf.o: /usr/include/newlib.h /usr/include/_newlib_version.h
uge_ramprintf.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
uge_ramprintf.o: /usr/include/sys/features.h /usr/include/sys/cdefs.h
uge_ramprintf.o: /usr/include/machine/_default_types.h
uge_ramprintf.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
uge_ramprintf.o: /usr/include/machine/_types.h /usr/include/sys/lock.h
uge_ramprintf.o: /usr/include/sys/stdio.h /usr/include/stdlib.h
uge_ramprintf.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
uge_ramprintf.o: /usr/include/string.h /usr/include/sys/_locale.h
uge_ramprintf.o: /usr/include/strings.h /usr/include/sys/string.h
uge_ramprintf.o: uge_ramprintf.hpp
uge_z.o: /usr/include/math.h /usr/include/sys/reent.h /usr/include/_ansi.h
uge_z.o: /usr/include/newlib.h /usr/include/_newlib_version.h
uge_z.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
uge_z.o: /usr/include/sys/features.h /usr/include/sys/cdefs.h
uge_z.o: /usr/include/machine/_default_types.h /usr/include/sys/_types.h
uge_z.o: /usr/include/machine/_types.h /usr/include/sys/lock.h
uge_z.o: /usr/include/string.h /usr/include/sys/_locale.h
uge_z.o: /usr/include/strings.h /usr/include/sys/string.h
uge_z.o: /usr/include/assert.h uge_err.hpp uge_ramprintf.hpp gcstr.hpp
uge_z.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
uge_z.o: /usr/include/alloca.h uge_z.hpp /usr/include/stdint.h
uge_z.o: /usr/include/sys/_intsup.h /usr/include/sys/_stdint.h
ztest.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
ztest.o: /usr/include/_newlib_version.h /usr/include/sys/config.h
ztest.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
ztest.o: /usr/include/sys/cdefs.h /usr/include/machine/_default_types.h
ztest.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
ztest.o: /usr/include/machine/_types.h /usr/include/sys/lock.h
ztest.o: /usr/include/sys/stdio.h /usr/include/stdlib.h
ztest.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
ztest.o: /usr/include/stdint.h /usr/include/sys/_intsup.h
ztest.o: /usr/include/sys/_stdint.h /usr/include/assert.h
ztest.o: /usr/include/string.h /usr/include/sys/_locale.h
ztest.o: /usr/include/strings.h /usr/include/sys/string.h gcstr.hpp
ztest.o: uge_ramprintf.hpp uge_z.hpp
