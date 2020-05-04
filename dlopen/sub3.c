#include <time.h>
#include <stdio.h>

void sub3(void)
{
	printf("sub3 clock=%p\n", clock);
	printf("sub3 clock()=%d\n", (int)clock());
}
