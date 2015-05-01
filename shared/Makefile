CFLAGS=-g -fsanitize=address
test: a.c b.so
	gcc a.c -o test ./b.so $(CFLAGS) -ldl

b.so: b.c
	gcc -fPIC -shared -o b.so b.c $(CFLAGS)

clean:
	rm test b.so
