#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define USE_DLOPEN

#ifndef USE_DLOPEN
void sub_free(void*);
#endif


int main()
{
	puts("main");
	char *p = malloc(123);
#ifdef USE_DLOPEN
	puts("dlopen");
	void *h = dlopen("./b.so", RTLD_LAZY);
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
	return 0;
}
