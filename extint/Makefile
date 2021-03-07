TARGET=add256.o mul256.o

ALL: $(TARGET)

CXX?=clang++
CFLAGS=-Wall -Wextra -Ofast -mbmi2

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.ll
	$(CXX) -c -o $@ $< $(CFLAGS)

clean:
	$(RM) -rf *.o *.bc
