CFLAGS=-Wall -Wextra -O3
lib/add.so: add.cpp
	gcc $(CFLAGS) -fPIC -shared -o $@ -g $<

# linux
test: test.cpp lib/add.so
	gcc -o test test.cpp lib/add.so

run: test
#	env LD_RUN_PATH="./lib" ./test
	./test

# make dll by gcc
mingw0:
	gcc $(CFLAGS) -shared -o lib/add.so add.cpp -Wl,--out-implib,lib/libadd.a
	gcc -o test test.cpp -ladd -L./lib
	env PATH=$(PATH):./lib ./test.exe

mingw1:
	gcc $(CFLAGS) -shared -o lib/add.dll add.cpp -Wl,--out-implib,lib/libadd.a
	gcc -o test test.cpp -ladd -L./lib
	env PATH=$(PATH):./lib ./test.exe

# make dll by vc
lib/add.dll: add.cpp
	cd lib
	cl /LD ../add.cpp

mingw2:
	gcc -o test test.cpp -ladd -L./lib
	env PATH=$(PATH):./lib ./test.exe

clean:
	rm -rf test lib/add.* *.dll lib/libadd.a *.exe
