#include <stdio.h>

extern "C" void cstr();

int main()
{
	puts("main");
	cstr();
	puts("end");
}
