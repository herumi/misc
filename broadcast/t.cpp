#include <stdio.h>

extern "C" double b[8];
extern "C" double c[8];
extern "C" void f();
extern "C" void g();

void put(const double *p)
{
	for (int i = 0; i < 8; i++) {
		printf("%f ", p[i]);
	}
	printf("\n");
}

int main()
{
	f();
	put(b);
	g();
	put(c);
}
