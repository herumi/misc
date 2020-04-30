#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
	void *p;
	puts("main-dl");
	printf("malloc=%p, free=%p\n", malloc, free);
	p = malloc(123);
#ifdef DEEP
	puts("deep");
	void *h = dlopen("./sub.so", RTLD_LAZY | RTLD_DEEPBIND);
#else
	void *h = dlopen("./sub.so", RTLD_LAZY);
#endif
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
	return 0;
}
