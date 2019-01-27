#include <stdio.h>
#include "lib.h"

extern void GoFunc1();
extern void GoFunc2(const void *buf, int n);
extern void GoCallF(void *buf, int n);

void fill(void *buf, int n)
{
	char *p = (char*)buf;
	for (int i = 0; i < n; i++) {
		p[i] = i;
	}
}
void ggg(void f(void *, int), void *buf, int n)
{
	puts("ggg callback");
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
	if (buf == 0) return;
	const unsigned char *p = (const unsigned char *)buf;
	for (int i = 0; i < n; i++) {
		printf("%02x ", p[i]);
	}
	printf("\n");
	GoFunc2(buf, n);
}

#define S_BUF_SIZE 8
static char s_buf[S_BUF_SIZE];

void callGoCallF()
{
	GoCallF(s_buf, S_BUF_SIZE);
}

void putBuf()
{
	printf("putBuf ");
	for (int i = 0; i < S_BUF_SIZE; i++) {
		printf("%02x ", (unsigned char)s_buf[i]);
	}
	printf("\n");
}

FuncType s_f;
void setCallback(FuncType f)
{
	s_f = f;
}
