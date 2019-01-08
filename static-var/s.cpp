#define DLL_EXPORT
#include "var.hpp"

void put()
{
	printf("sub msg=%p\n", X<int>::get());
}

void incX()
{
	puts("call incX");
	X<int>().inc();
	printf("sub x=%d\n", X<int>::x);
}

int XX::x = 4;
