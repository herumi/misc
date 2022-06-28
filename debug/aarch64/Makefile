CFLAGS+=-Wall -Wextra -O2 -g

OBJS=test.o func.o
all: test.exe

%.o: %.cpp
	$(PRE)$(CXX) -c -o $@ $< $(CFLAGS)
%.o: %.s
	$(PRE)$(CXX) -c -o $@ $<

test.exe: $(OBJS)
	$(PRE)$(CXX) -o $@ $(OBJS)

test: test.exe
	./test.exe

result: test.exe
	@gdb -nx -q -batch -x script.txt ./test.exe | grep QQQ

clean:
	$(RM) *.o *.exe

.PHONY: clean
