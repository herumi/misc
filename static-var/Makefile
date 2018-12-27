test1: t.cpp s.cpp
	g++ t.cpp s.cpp -o test1


test: test1 test2
	echo test1
	./test1
	echo test2
	env LD_PRELOAD=./sub.so ./test2

sub.so: s.cpp
	g++ -O2 -fPIC -shared -o $@ -g $<

test2: t.cpp sub.so
	g++ t.cpp -o $@ ./sub.so

clean:
	rm -rf test1 test2 sub.so

