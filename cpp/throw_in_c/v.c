#include <stdio.h>

static void g(void f())
{
	puts("g");
	f();
}

void (*get())(void f())
{
	puts("get");
	return g;
}
