CFLAGS=-g -Wall -Wextra

test: test.exe
	./$<

test.exe: test.o gen.o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.S
	$(CXX) -c -o $@ $<

gen.S: gen.py Makefile
	python3 gen.py -m gas > $@

clean:
	rm -rf *.o *.exe *.S

.PHONY: test clean
