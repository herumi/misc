#include <stdio.h>

extern void GoFunc1();
extern void GoFunc2(const void *buf, int n);

void fill(void *buf, int n)
{
	char *p = (char*)buf;
	for (int i = 0; i < n; i++) {
		p[i] = i;
	}
}
void ggg(void f(void *, int), void *buf, int n)
{
	puts("callback");
	f(buf, n);
}
void hhh()
{
	GoFunc1();
}
int fff()
{
	return 123;
}

void putput(const void *buf, int n)
{
	printf("putput buf=%p, n=%d\n", buf, n);
	const unsigned char *p = (const unsigned char *)buf;
	for (int i = 0; i < n; i++) {
		printf("%02x ", p[i]);
	}
	printf("\n");
	GoFunc2(buf, n);
}
