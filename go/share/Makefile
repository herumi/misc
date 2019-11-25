CFLAGS=-Wall -Wextra -O3 -I./include

add.o: add.cpp
	gcc $(CFLAGS) -fPIC -c -o $@ -g $<

lib/libadd.so: add.cpp
	gcc $(CFLAGS) -fPIC -shared -o $@ -g $<

lib/libadd.a: add.o
	ar r lib/libadd.a add.o

# linux
test: test.cpp lib/libadd.so
	gcc -o test $(CFLAGS) test.cpp -ladd -L./lib

run: test
#	env LD_RUN_PATH="./lib" ./test
	./test

# make dll by gcc
mingw0:
	gcc $(CFLAGS) -shared -o lib/add.so add.cpp -Wl,--out-implib,lib/libadd.a
	gcc -o test $(CFLAGS) test.cpp -ladd -L./lib
	env PATH=$(PATH):./lib ./test.exe

mingw1:
	gcc $(CFLAGS) -shared -o lib/add.dll add.cpp -Wl,--out-implib,lib/libadd.a
	gcc -o test $(CFLAGS) test.cpp -ladd -L./lib
	env PATH=$(PATH):./lib ./test.exe

# make dll by vc
lib/add.dll: add.cpp
	cd lib
	cl /LD ../add.cpp -I../include

mingw2:
	gcc -o test $(CFLAGS) test.cpp -ladd -L./lib
	env PATH=$(PATH):./lib ./test.exe

test_go:
	env PATH=$(PATH):./lib CGO_CFLAGS="-DAAA=5" go run test.go

clean:
	rm -rf test lib/add.* *.dll lib/libadd.a *.exe
