#include <stdio.h>

void ggg(void f(void *, int), void *buf, int n);

int fff();

void GoFunc1()
{
	puts("test GoFunc1");
}

void GoFunc2(const void *buf, int n)
{
	printf("test GoFunc1 buf=%p, n=%d\n", buf, n);
}

int main()
{
	printf("fff=%d\n", fff());
}
