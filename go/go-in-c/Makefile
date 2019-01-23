all: liblib.a libtest

%.o: %.c
	gcc -c -O2 -fPIC -Wall -Wextra -o $@ -g $<

liblib.a: lib.o
	ar r $@ $<

libtest: libtest.o liblib.a
	gcc -o $@ $< -L./ -llib

test: libtest
	./libtest

test_go: liblib.a
	go run main.go

clean:
	rm -rf *.o *.a libtest

