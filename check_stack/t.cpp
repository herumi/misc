#include <stdio.h>

extern "C" const void* f();

extern "C" void fff(void*);

char ggg(int x)
{
	char buf[8192];
	buf[0] = x;
	fff(buf);
	return buf[0];
}

int main()
{
	const void *p = f();
	printf("p=%p\n", p);
	ggg(3);
}
