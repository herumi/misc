CFLAGS=-g -O2
test1: test.c lib1.so lib2.so
	gcc -o $@ $< $(CFLAGS) ./lib1.so ./lib2.so

test2: test.c lib1.so lib2.so
	gcc -o $@ $< $(CFLAGS) ./lib2.so ./lib1.so

lib1.so: ./lib.c
	gcc -o $@ $< $(CFLAGS) -fPIC -shared -DLIB=1

lib2.so: ./lib.c
	gcc -o $@ $< $(CFLAGS) -fPIC -shared -DLIB=2

test: test1 test2
	echo test1
	./test1
	echo test2
	./test2

clean:
	rm -rf *.so test1 test2

