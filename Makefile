CPP=g++
CFLAGS=-g

OBJS=\
   uge_z.o \
   uge_q.o \
   uge_ramprintf.o

all: ztest qtest uge

tar:
	rm -f ../`basename $$(git rev-parse --show-toplevel)`.*.tar.gz
	git ls-files | tar -czv -T - -f /tmp/`basename $$(git rev-parse --show-toplevel)`.`date -u "+%Y%m%d_%H%M%S"`.tar.gz

ztest: ztest.o $(OBJS)
	$(CPP) $(CFLAGS) ztest.o $(OBJS) -o ztest

qtest: qtest.o $(OBJS)
	$(CPP) $(CFLAGS) qtest.o $(OBJS) -o qtest

uge: uge.o $(OBJS)
	$(CPP) $(CFLAGS) uge.o $(OBJS) -o uge

%.o: %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@

%.o: %.cpp %.hpp
	$(CPP) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o ztest qtest uge

.PHONY: depend
depend:
	makedepend *.cpp

# DO NOT DELETE

qtest.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
qtest.o: /usr/include/features.h /usr/include/features-time64.h
qtest.o: /usr/include/bits/wordsize.h /usr/include/bits/timesize.h
qtest.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
qtest.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
qtest.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
qtest.o: /usr/include/bits/time64.h /usr/include/bits/types/__fpos_t.h
qtest.o: /usr/include/bits/types/__mbstate_t.h
qtest.o: /usr/include/bits/types/__fpos64_t.h
qtest.o: /usr/include/bits/types/__FILE.h /usr/include/bits/types/FILE.h
qtest.o: /usr/include/bits/types/struct_FILE.h
qtest.o: /usr/include/bits/types/cookie_io_functions_t.h
qtest.o: /usr/include/bits/stdio_lim.h /usr/include/bits/floatn.h
qtest.o: /usr/include/bits/floatn-common.h /usr/include/stdlib.h
qtest.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
qtest.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
qtest.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
qtest.o: /usr/include/bits/types/timer_t.h /usr/include/bits/stdint-intn.h
qtest.o: /usr/include/endian.h /usr/include/bits/endian.h
qtest.o: /usr/include/bits/endianness.h /usr/include/bits/byteswap.h
qtest.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
qtest.o: /usr/include/bits/select.h /usr/include/bits/types/sigset_t.h
qtest.o: /usr/include/bits/types/__sigset_t.h
qtest.o: /usr/include/bits/types/struct_timeval.h
qtest.o: /usr/include/bits/types/struct_timespec.h
qtest.o: /usr/include/bits/pthreadtypes.h
qtest.o: /usr/include/bits/thread-shared-types.h
qtest.o: /usr/include/bits/pthreadtypes-arch.h
qtest.o: /usr/include/bits/atomic_wide_counter.h
qtest.o: /usr/include/bits/struct_mutex.h /usr/include/bits/struct_rwlock.h
qtest.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
qtest.o: /usr/include/stdint.h /usr/include/bits/wchar.h
qtest.o: /usr/include/bits/stdint-uintn.h /usr/include/bits/stdint-least.h
qtest.o: /usr/include/assert.h /usr/include/string.h
qtest.o: /usr/include/bits/types/locale_t.h
qtest.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
qtest.o: uge_ramprintf.hpp gcstr.hpp uge_q.hpp uge_z.hpp
uge.o: /usr/include/ctype.h /usr/include/features.h
uge.o: /usr/include/features-time64.h /usr/include/bits/wordsize.h
uge.o: /usr/include/bits/timesize.h /usr/include/stdc-predef.h
uge.o: /usr/include/sys/cdefs.h /usr/include/bits/long-double.h
uge.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
uge.o: /usr/include/bits/typesizes.h /usr/include/bits/time64.h
uge.o: /usr/include/bits/endian.h /usr/include/bits/endianness.h
uge.o: /usr/include/bits/types/locale_t.h
uge.o: /usr/include/bits/types/__locale_t.h /usr/include/errno.h
uge.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
uge.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
uge.o: /usr/include/asm-generic/errno-base.h /usr/include/fcntl.h
uge.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
uge.o: /usr/include/bits/types/struct_timespec.h
uge.o: /usr/include/bits/types/time_t.h /usr/include/bits/stat.h
uge.o: /usr/include/bits/struct_stat.h /usr/include/stdint.h
uge.o: /usr/include/bits/libc-header-start.h /usr/include/bits/wchar.h
uge.o: /usr/include/bits/stdint-intn.h /usr/include/bits/stdint-uintn.h
uge.o: /usr/include/bits/stdint-least.h /usr/include/stdio.h
uge.o: /usr/include/bits/types/__fpos_t.h
uge.o: /usr/include/bits/types/__mbstate_t.h
uge.o: /usr/include/bits/types/__fpos64_t.h /usr/include/bits/types/__FILE.h
uge.o: /usr/include/bits/types/FILE.h /usr/include/bits/types/struct_FILE.h
uge.o: /usr/include/bits/types/cookie_io_functions_t.h
uge.o: /usr/include/bits/stdio_lim.h /usr/include/bits/floatn.h
uge.o: /usr/include/bits/floatn-common.h /usr/include/stdlib.h
uge.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
uge.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
uge.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/timer_t.h
uge.o: /usr/include/endian.h /usr/include/bits/byteswap.h
uge.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
uge.o: /usr/include/bits/select.h /usr/include/bits/types/sigset_t.h
uge.o: /usr/include/bits/types/__sigset_t.h
uge.o: /usr/include/bits/types/struct_timeval.h
uge.o: /usr/include/bits/pthreadtypes.h
uge.o: /usr/include/bits/thread-shared-types.h
uge.o: /usr/include/bits/pthreadtypes-arch.h
uge.o: /usr/include/bits/atomic_wide_counter.h
uge.o: /usr/include/bits/struct_mutex.h /usr/include/bits/struct_rwlock.h
uge.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
uge.o: /usr/include/string.h /usr/include/strings.h /usr/include/termios.h
uge.o: /usr/include/bits/termios.h /usr/include/bits/termios-struct.h
uge.o: /usr/include/bits/termios-c_cc.h /usr/include/bits/termios-c_iflag.h
uge.o: /usr/include/bits/termios-c_oflag.h /usr/include/bits/termios-baud.h
uge.o: /usr/include/bits/termios-c_cflag.h
uge.o: /usr/include/bits/termios-c_lflag.h /usr/include/bits/termios-tcflow.h
uge.o: /usr/include/bits/termios-misc.h /usr/include/sys/ttydefaults.h
uge.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
uge.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
uge.o: /usr/include/bits/getopt_posix.h /usr/include/bits/getopt_core.h
uge.o: /usr/include/bits/unistd_ext.h gcstr.hpp uge_q.hpp uge_z.hpp
uge_q.o: /usr/include/math.h /usr/include/bits/libc-header-start.h
uge_q.o: /usr/include/features.h /usr/include/features-time64.h
uge_q.o: /usr/include/bits/wordsize.h /usr/include/bits/timesize.h
uge_q.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
uge_q.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
uge_q.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
uge_q.o: /usr/include/bits/time64.h /usr/include/bits/math-vector.h
uge_q.o: /usr/include/bits/libm-simd-decl-stubs.h /usr/include/bits/floatn.h
uge_q.o: /usr/include/bits/floatn-common.h
uge_q.o: /usr/include/bits/flt-eval-method.h /usr/include/bits/fp-logb.h
uge_q.o: /usr/include/bits/fp-fast.h /usr/include/bits/mathcalls-macros.h
uge_q.o: /usr/include/bits/mathcalls-helper-functions.h
uge_q.o: /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls-narrow.h
uge_q.o: /usr/include/bits/iscanonical.h /usr/include/string.h
uge_q.o: /usr/include/bits/types/locale_t.h
uge_q.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
uge_q.o: /usr/include/assert.h uge_err.hpp uge_ramprintf.hpp gcstr.hpp
uge_q.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
uge_q.o: /usr/include/bits/waitstatus.h /usr/include/sys/types.h
uge_q.o: /usr/include/bits/types/clock_t.h
uge_q.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
uge_q.o: /usr/include/bits/types/timer_t.h /usr/include/bits/stdint-intn.h
uge_q.o: /usr/include/endian.h /usr/include/bits/endian.h
uge_q.o: /usr/include/bits/endianness.h /usr/include/bits/byteswap.h
uge_q.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
uge_q.o: /usr/include/bits/select.h /usr/include/bits/types/sigset_t.h
uge_q.o: /usr/include/bits/types/__sigset_t.h
uge_q.o: /usr/include/bits/types/struct_timeval.h
uge_q.o: /usr/include/bits/types/struct_timespec.h
uge_q.o: /usr/include/bits/pthreadtypes.h
uge_q.o: /usr/include/bits/thread-shared-types.h
uge_q.o: /usr/include/bits/pthreadtypes-arch.h
uge_q.o: /usr/include/bits/atomic_wide_counter.h
uge_q.o: /usr/include/bits/struct_mutex.h /usr/include/bits/struct_rwlock.h
uge_q.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h uge_q.hpp
uge_q.o: /usr/include/stdint.h /usr/include/bits/wchar.h
uge_q.o: /usr/include/bits/stdint-uintn.h /usr/include/bits/stdint-least.h
uge_q.o: uge_z.hpp
uge_ramprintf.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
uge_ramprintf.o: /usr/include/features.h /usr/include/features-time64.h
uge_ramprintf.o: /usr/include/bits/wordsize.h /usr/include/bits/timesize.h
uge_ramprintf.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
uge_ramprintf.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
uge_ramprintf.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
uge_ramprintf.o: /usr/include/bits/time64.h
uge_ramprintf.o: /usr/include/bits/types/__fpos_t.h
uge_ramprintf.o: /usr/include/bits/types/__mbstate_t.h
uge_ramprintf.o: /usr/include/bits/types/__fpos64_t.h
uge_ramprintf.o: /usr/include/bits/types/__FILE.h
uge_ramprintf.o: /usr/include/bits/types/FILE.h
uge_ramprintf.o: /usr/include/bits/types/struct_FILE.h
uge_ramprintf.o: /usr/include/bits/types/cookie_io_functions_t.h
uge_ramprintf.o: /usr/include/bits/stdio_lim.h /usr/include/bits/floatn.h
uge_ramprintf.o: /usr/include/bits/floatn-common.h /usr/include/stdlib.h
uge_ramprintf.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
uge_ramprintf.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
uge_ramprintf.o: /usr/include/bits/types/clockid_t.h
uge_ramprintf.o: /usr/include/bits/types/time_t.h
uge_ramprintf.o: /usr/include/bits/types/timer_t.h
uge_ramprintf.o: /usr/include/bits/stdint-intn.h /usr/include/endian.h
uge_ramprintf.o: /usr/include/bits/endian.h /usr/include/bits/endianness.h
uge_ramprintf.o: /usr/include/bits/byteswap.h
uge_ramprintf.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
uge_ramprintf.o: /usr/include/bits/select.h
uge_ramprintf.o: /usr/include/bits/types/sigset_t.h
uge_ramprintf.o: /usr/include/bits/types/__sigset_t.h
uge_ramprintf.o: /usr/include/bits/types/struct_timeval.h
uge_ramprintf.o: /usr/include/bits/types/struct_timespec.h
uge_ramprintf.o: /usr/include/bits/pthreadtypes.h
uge_ramprintf.o: /usr/include/bits/thread-shared-types.h
uge_ramprintf.o: /usr/include/bits/pthreadtypes-arch.h
uge_ramprintf.o: /usr/include/bits/atomic_wide_counter.h
uge_ramprintf.o: /usr/include/bits/struct_mutex.h
uge_ramprintf.o: /usr/include/bits/struct_rwlock.h /usr/include/alloca.h
uge_ramprintf.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
uge_ramprintf.o: /usr/include/bits/types/locale_t.h
uge_ramprintf.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
uge_ramprintf.o: uge_ramprintf.hpp
uge_z.o: /usr/include/math.h /usr/include/bits/libc-header-start.h
uge_z.o: /usr/include/features.h /usr/include/features-time64.h
uge_z.o: /usr/include/bits/wordsize.h /usr/include/bits/timesize.h
uge_z.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
uge_z.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
uge_z.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
uge_z.o: /usr/include/bits/time64.h /usr/include/bits/math-vector.h
uge_z.o: /usr/include/bits/libm-simd-decl-stubs.h /usr/include/bits/floatn.h
uge_z.o: /usr/include/bits/floatn-common.h
uge_z.o: /usr/include/bits/flt-eval-method.h /usr/include/bits/fp-logb.h
uge_z.o: /usr/include/bits/fp-fast.h /usr/include/bits/mathcalls-macros.h
uge_z.o: /usr/include/bits/mathcalls-helper-functions.h
uge_z.o: /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls-narrow.h
uge_z.o: /usr/include/bits/iscanonical.h /usr/include/string.h
uge_z.o: /usr/include/bits/types/locale_t.h
uge_z.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
uge_z.o: /usr/include/assert.h /usr/include/inttypes.h /usr/include/stdint.h
uge_z.o: /usr/include/bits/wchar.h /usr/include/bits/stdint-intn.h
uge_z.o: /usr/include/bits/stdint-uintn.h /usr/include/bits/stdint-least.h
uge_z.o: uge_err.hpp uge_ramprintf.hpp gcstr.hpp /usr/include/stdlib.h
uge_z.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
uge_z.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
uge_z.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
uge_z.o: /usr/include/bits/types/timer_t.h /usr/include/endian.h
uge_z.o: /usr/include/bits/endian.h /usr/include/bits/endianness.h
uge_z.o: /usr/include/bits/byteswap.h /usr/include/bits/uintn-identity.h
uge_z.o: /usr/include/sys/select.h /usr/include/bits/select.h
uge_z.o: /usr/include/bits/types/sigset_t.h
uge_z.o: /usr/include/bits/types/__sigset_t.h
uge_z.o: /usr/include/bits/types/struct_timeval.h
uge_z.o: /usr/include/bits/types/struct_timespec.h
uge_z.o: /usr/include/bits/pthreadtypes.h
uge_z.o: /usr/include/bits/thread-shared-types.h
uge_z.o: /usr/include/bits/pthreadtypes-arch.h
uge_z.o: /usr/include/bits/atomic_wide_counter.h
uge_z.o: /usr/include/bits/struct_mutex.h /usr/include/bits/struct_rwlock.h
uge_z.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h uge_z.hpp
ztest.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
ztest.o: /usr/include/features.h /usr/include/features-time64.h
ztest.o: /usr/include/bits/wordsize.h /usr/include/bits/timesize.h
ztest.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
ztest.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
ztest.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
ztest.o: /usr/include/bits/time64.h /usr/include/bits/types/__fpos_t.h
ztest.o: /usr/include/bits/types/__mbstate_t.h
ztest.o: /usr/include/bits/types/__fpos64_t.h
ztest.o: /usr/include/bits/types/__FILE.h /usr/include/bits/types/FILE.h
ztest.o: /usr/include/bits/types/struct_FILE.h
ztest.o: /usr/include/bits/types/cookie_io_functions_t.h
ztest.o: /usr/include/bits/stdio_lim.h /usr/include/bits/floatn.h
ztest.o: /usr/include/bits/floatn-common.h /usr/include/stdlib.h
ztest.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
ztest.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
ztest.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
ztest.o: /usr/include/bits/types/timer_t.h /usr/include/bits/stdint-intn.h
ztest.o: /usr/include/endian.h /usr/include/bits/endian.h
ztest.o: /usr/include/bits/endianness.h /usr/include/bits/byteswap.h
ztest.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
ztest.o: /usr/include/bits/select.h /usr/include/bits/types/sigset_t.h
ztest.o: /usr/include/bits/types/__sigset_t.h
ztest.o: /usr/include/bits/types/struct_timeval.h
ztest.o: /usr/include/bits/types/struct_timespec.h
ztest.o: /usr/include/bits/pthreadtypes.h
ztest.o: /usr/include/bits/thread-shared-types.h
ztest.o: /usr/include/bits/pthreadtypes-arch.h
ztest.o: /usr/include/bits/atomic_wide_counter.h
ztest.o: /usr/include/bits/struct_mutex.h /usr/include/bits/struct_rwlock.h
ztest.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
ztest.o: /usr/include/stdint.h /usr/include/bits/wchar.h
ztest.o: /usr/include/bits/stdint-uintn.h /usr/include/bits/stdint-least.h
ztest.o: /usr/include/assert.h /usr/include/string.h
ztest.o: /usr/include/bits/types/locale_t.h
ztest.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
ztest.o: gcstr.hpp uge_ramprintf.hpp uge_z.hpp
