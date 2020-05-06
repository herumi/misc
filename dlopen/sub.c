#include <stdio.h>
#include <stdlib.h>

void sub_free(void *p)
{
	puts("sub_free");
	printf("malloc=%p, free=%p in sub\n", malloc, free);
	printf("p=%p\n", p);
	free(p);
}
