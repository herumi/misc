test: test.exe
	./test.exe

CFLAGS=-g -Wall -Wextra

test.exe: test.cpp gen.o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.S
	$(CXX) -c -o $@ $<

gen.S: gen.py
	python3 gen.py -m gas > gen.S

clean:
	rm -rf *.o *.exe *.S

.PHONY: test clean
