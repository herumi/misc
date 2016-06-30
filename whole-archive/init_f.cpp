#include <stdio.h>

extern "C" void (*g_f)();

extern "C" void f()
{
	puts("f is called");
}

static struct X {
	X()
	{
		puts("g_f is set");
		g_f = &f;
	}
} x;
