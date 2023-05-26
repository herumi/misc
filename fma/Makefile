test: fma_test.exe
	./fma_test.exe

fma_test.exe: fma_test.o fma.o
	$(CXX) -o $@ $^

fma_test.o: fma_test.cpp func.h
	$(CXX) -c -o $@ $< -DNDEBUG -O2 -I ../../cybozulib/include

func.h: gen.py
	python3 gen.py -header > $@

fma.o: fma.S
	$(CXX) -c -o $@ $<

fma.S: gen.py
	python3 gen.py -m gas > $@

clean:
	rm -f *.S *.o *.exe

