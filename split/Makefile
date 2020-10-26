all = test1 test2

test1: inst.cpp t.h t.cpp impl.h
	$(CXX) t.cpp -o test1

test2: inst.o t.cpp t.h
	$(CXX) t.cpp inst.o -DUSE_PRECOMPILE_HEADER -o test2

inst.o: inst.cpp impl.h t.h
	$(CXX) -c inst.cpp

test: test1 test2
	echo test1
	./test1
	echo test2
	./test2

clean:
	rm -rf *.o test1 test2
