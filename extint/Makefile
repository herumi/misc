TARGET=add256.o mul256.o

ALL: $(TARGET)

CXX=clang++-11
CFLAGS=-Wall -Wextra -Ofast -march=native -mbmi2

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)
