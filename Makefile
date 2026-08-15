CXX ?= g++
CXXFLAGS ?= -O3 -g
DEPFLAGS=-MMD -MP

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DESTDIR ?=
EXEEXT ?= $(if $(findstring mingw,$(CXX)),.exe,)
UGE_EXE=uge$(EXEEXT)

VERSION_H=version.h
VERSION_SCRIPT=gen_version_h.pl
GIT_VERSION_DEPS=$(wildcard .git/HEAD .git/packed-refs .git/refs/tags/*)

OBJS=\
   uge_n.o \
   uge_q.o \
   uge_c.o \
   uge_ramprintf.o

PROGS=ntest ztest qtest ctest cetest
TEST_PROGS=regression
PROG_OBJS=ntest.o ztest.o qtest.o ctest.o cetest.o uge.o
DEPS=$(OBJS:.o=.d) uge_z.d uge_ce.d $(PROG_OBJS:.o=.d)

all: $(PROGS) $(UGE_EXE)

$(VERSION_H): $(VERSION_SCRIPT) $(GIT_VERSION_DEPS)
	./$(VERSION_SCRIPT) $(VERSION_H)

version:
	./$(VERSION_SCRIPT) $(VERSION_H)

# Generate version.h immediately before compiling uge.cpp.  This updates the
# timestamp fallback whenever a compile actually occurs, while Git HEAD/tag
# changes also make uge.o stale in a working tree.
uge.o: uge.cpp $(VERSION_SCRIPT) $(GIT_VERSION_DEPS)
	./$(VERSION_SCRIPT) $(VERSION_H)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

tar:
	rm -f ../`basename $$(git rev-parse --show-toplevel)`.*.tar.gz
	git ls-files | tar -czv -T - -f /tmp/`basename $$(git rev-parse --show-toplevel)`.`date -u "+%Y%m%d_%H%M%S"`.tar.gz

ntest: ntest.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

ztest: ztest.o uge_z.o uge_n.o uge_ramprintf.o
	$(CXX) $(CXXFLAGS) $^ -o $@

qtest: qtest.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(UGE_EXE): uge.o uge_ce.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

ctest: ctest.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

cetest: cetest.o uge_ce.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

regression: tests/regression.cpp $(OBJS) uge_z.o uge_ce.o
	$(CXX) $(CXXFLAGS) -I. $^ -o $@

test: regression $(UGE_EXE)
	./regression
	UGE=./$(UGE_EXE) ./tests/uge_regression.sh
	./tests/version_regression.sh
	./tests/install_regression.sh

sanitize:
	@status=0; \
	$(MAKE) clean; \
	$(MAKE) test CXXFLAGS='-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer' || status=$$?; \
	$(MAKE) clean; \
	exit $$status

install: $(UGE_EXE)
	install -d '$(DESTDIR)$(BINDIR)'
	install -m 0755 $(UGE_EXE) '$(DESTDIR)$(BINDIR)/uge$(EXEEXT)'

uninstall:
	rm -f '$(DESTDIR)$(BINDIR)/uge$(EXEEXT)'

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: all version test sanitize install uninstall clean
clean:
	rm -f *.o *.d $(PROGS) uge uge.exe $(TEST_PROGS) $(VERSION_H)
