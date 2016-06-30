#include <stdio.h>

extern "C" {

void (*g_f)();

}

extern "C" void sub()
{
	if (g_f == 0) {
		puts("not inited");
	} else {
		puts("call f");
		g_f();
	}
}
