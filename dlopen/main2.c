#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>

int main()
{
	void *p;
	puts("main2");
	printf("clock=%p\n", clock);
	printf("clock()=%d\n", (int)clock());
#ifdef DEEP
	puts("deep");
	void *h = dlopen("./sub3.so", RTLD_LAZY | RTLD_DEEPBIND);
#else
	void *h = dlopen("./sub3.so", RTLD_LAZY);
#endif
	if (h == NULL) {
		perror("dlopen");
		return 1;
	}
	void (*f)(void) = dlsym(h, "sub3");
	if (f == NULL) {
		perror("dlsym");
		return 1;
	}
	puts("call sub3 in sub3.c");
	f();
	puts("dlclose");
	dlclose(h);
	return 0;
}
