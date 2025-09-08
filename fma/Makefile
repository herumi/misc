test: fma_test.exe
	@echo OP=$(OP)
	./fma_test.exe

fma_test.exe: fma_test.o fma.o
	$(CXX) -o $@ $^

fma_test.o: fma_test.cpp func.h
	$(CXX) -c -o $@ $< -DNDEBUG -O2 -g -I ../../cybozulib/include

func.h: gen.py
	python3 gen.py -header > $@

fma.o: fma.S
	$(CXX) -c -o $@ $<

OP?=fma
fma.S: gen.py
	python3 gen.py -m gas -op $(OP) > $@

clean:
	rm -f *.S *.o *.exe func.h

