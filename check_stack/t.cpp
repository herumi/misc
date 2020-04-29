#include <stdio.h>

extern "C" const void* f();

extern "C" void fff(const void*);

char ggg()
{
	char buf[64];
	fff(buf);
	return buf[0];
}

int main()
{
	f();
	ggg();
	puts("ok");
}
