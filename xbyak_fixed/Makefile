all: t2.exe t4.exe
CFLAGS+=-I../../xbyak -I../../cybozulib/include -Wall -Wextra

%.o: %.asm
	nasm -f elf64 $<

t1.exe: t1.cpp
	$(CXX) $(CFLAGS) -o $@ $<
t2.exe: t2.cpp t1-out.o
	$(CXX) $(CFLAGS) -o $@ $?

t1-out.asm: t1.exe
	./t1.exe $@

t3.exe: t3.cpp
	$(CXX) $(CFLAGS) -o $@ $<
t4.exe: t4.cpp t3-out.o
	$(CXX) $(CFLAGS) -o $@ $?

t3-out.asm: t3.exe
	./t3.exe $@

test: t2.exe t4.exe
	./t2.exe
	./t4.exe

clean:
	rm -rf *.exe *.o *.asm
