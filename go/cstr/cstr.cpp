#include <stdio.h>

struct X {
	char *p;
	X()
		: p(new char[123])
	{
		puts("X cstr");
	}
	~X()
	{
		puts("X dstr");
		delete[] p;
	}
};

extern "C" void cstr()
{
	static X x;
}

