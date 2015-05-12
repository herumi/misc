CFLAGS=-g -fsanitize=address
MOPT= -include ../mymalloc.h
test: a.c b.so mymalloc.so
	gcc a.c -g -o test ./b.so $(MOPT) -ldl -DUSE_DLOPEN ./mymalloc.so

mymalloc.so: ../mymalloc.c
	gcc -O2 -fPIC -shared -o $@ -g $<

prem.so: ../mymalloc.c
	gcc -O2 -fPIC -shared -o $@ -g $< -DMYMALLOC_PRELOAD

b.so: b.c
	gcc -fPIC -shared -o b.so b.c $(MOPT) -DUSE_DLOPEN

bb.so: b.c
	gcc -fPIC -shared -o bb.so b.c -g $(MOPT)
tt: a.c bb.so prem.so
	gcc a.c -o tt b.c -g ./prem.so -ldl

run: tt
#	env LD_PRELOAD=./prem.so ./tt
	./tt

clean:
	rm -rf test b.so bb.so tt mymalloc.so

