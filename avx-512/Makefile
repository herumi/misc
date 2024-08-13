CFLAGS=-g -Wall -Wextra -mavx512f -mavx512ifma

test: test.exe
	./$<

bench_test: bench_test.exe
	./$<

test.exe: test.o gen.o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.S
	$(CXX) -c -o $@ $<

gen.S: gen.py Makefile
	python3 gen.py -m gas > $@

bench.S: bench.py Makefile
	python3 bench.py -m gas > $@

bench_test.exe: bench_test.o bench.o
	$(CXX) -o $@ $^

clean:
	rm -rf *.o *.exe *.S

.PHONY: test clean
