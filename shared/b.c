#include <stdio.h>
#include <stdlib.h>

void sub_free(void *p)
{
	puts("sub_free");
	free(p);
}
