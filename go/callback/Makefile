%.o: %.c
	gcc -c -O2 -fPIC -Wall -Wextra -o $@ -g $<

liblib.a: lib.o
	ar r $@ $<

test: liblib.a main.go callback.go sub.go
	go run main.go callback.go sub.go

clean:
	rm -rf *.a *.o callback
