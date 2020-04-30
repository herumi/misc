#include <stdio.h>
#include <stdlib.h>

void sub_free(void *p);

int main()
{
	void *p;
	puts("main");
	printf("malloc=%p, free=%p\n", malloc, free);
	p = malloc(123);
	sub_free(p);
	return 0;
}
