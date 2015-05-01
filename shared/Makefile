CFLAGS=-g -fsanitize=address
MOPT=../mymalloc.c -Dmalloc=mie_malloc -Dfree=mie_free -include ../mymalloc.h -DMYMALLOC
test: a.c b.so
	gcc a.c -o test ./b.so $(CFLAGS) -ldl

b.so: b.c
	gcc -fPIC -shared -o b.so b.c $(CFLAGS)

bb.so: b.c
	gcc -fPIC -shared -o bb.so b.c -g $(MOPT)
tt: a.c bb.so
	gcc a.c -o tt ./bb.so -g -ldl $(MOPT)

clean:
	rm -rf test b.so bb.so tt

