CXX ?= g++
CXXFLAGS ?= -O3 -g
DEPFLAGS=-MMD -MP

OBJS=\
   uge_n.o \
   uge_q.o \
   uge_c.o \
   uge_ramprintf.o

PROGS=ntest ztest qtest ctest uge
PROG_OBJS=$(PROGS:%=%.o)
DEPS=$(OBJS:.o=.d) uge_z.d $(PROG_OBJS:.o=.d)

all: $(PROGS)

tar:
	rm -f ../`basename $$(git rev-parse --show-toplevel)`.*.tar.gz
	git ls-files | tar -czv -T - -f /tmp/`basename $$(git rev-parse --show-toplevel)`.`date -u "+%Y%m%d_%H%M%S"`.tar.gz

ntest: ntest.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

ztest: ztest.o uge_z.o uge_n.o uge_ramprintf.o
	$(CXX) $(CXXFLAGS) $^ -o $@

qtest: qtest.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

uge: uge.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

ctest: ctest.o $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	rm -f *.o *.d $(PROGS)
