#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <dlfcn.h>

//#define USE_DLOPEN

#ifndef USE_DLOPEN
void sub_free(void*);
#endif

const char *soName = "./b.so";

void test_realloc()
{
	puts("test_realloc");
	char *p = malloc(1);
	p = realloc(p, 3);
	free(p);
}
int main()
{
	mie_init();
	puts("main");
	test_realloc();
	char *p = malloc(7);
#ifdef USE_DLOPEN
	puts("dlopen");
	void *h = dlopen(soName, RTLD_LAZY);
	if (h == NULL) {
		perror("dlopen");
		return 1;
	}
	puts("dlsym");
	void (*f)() = dlsym(h, "sub_free");
	if (f == NULL) {
		perror("dlsym");
		return 1;
	}
	puts("call");
	f(p);
	puts("dlclose");
	dlclose(h);
#else
	sub_free(p);
#endif
	{
		char *q = malloc(3);
		printf("malloc size=%zd\n", malloc_usable_size(q));
		q[3] = 'a';
		free(q);
	}
	return 0;
}
