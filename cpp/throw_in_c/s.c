#include <stdio.h>

extern void g();
extern void f();

void f()
{
	puts("call g");
	g();
}

