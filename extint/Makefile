TARGET=add256.o mul256.o

ALL: $(TARGET)

CXX?=clang++
CFLAGS=-Wall -Wextra -Ofast -mbmi2 -I ../../mcl/include -fpic -std=c++17

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.ll
	$(CXX) -c -o $@ $< $(CFLAGS)

bitint_test.exe: bitint_test.o
	$(CXX) -o $@ $< $(CFLAGS)

bitint_test.o: bitint_test.cpp bitint.hpp
	clang++-12 -c -o $@ $< $(CFLAGS)

bitint_test: bitint_test.exe
	./bitint_test.exe

clean:
	$(RM) -rf *.o *.bc *.exe
