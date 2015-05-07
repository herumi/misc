CFLAGS=-g -fsanitize=address
MOPT= -Dmalloc=mie_malloc -Drealloc=mie_realloc -Dcalloc=mie_calloc -Dfree=mie_free -Dstrdup=mie_strdup -include ../mymalloc.h
test: a.c b.so mymalloc.so
	gcc a.c -o test ./b.so $(MOPT) -ldl -DUSE_DLOPEN ./mymalloc.so

mymalloc.so: ../mymalloc.c
	gcc -fPIC -shared -o $@ -g $<

b.so: b.c
	gcc -fPIC -shared -o b.so b.c $(MOPT) -DUSE_DLOPEN

bb.so: b.c
	gcc -fPIC -shared -o bb.so b.c -g $(MOPT)
tt: a.c bb.so
	gcc a.c -o tt ./bb.so -g -ldl $(MOPT) ./mymalloc.so

clean:
	rm -rf test b.so bb.so tt mymalloc.so

