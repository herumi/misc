/*
	fast
	cl /EHsc /Ox /fp:fast slow_sin3.cpp

	second bench is slow
	cl /EHsc /Ox /fp:fast /arch:AVX slow_sin3.cpp
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

void bench()
{
	double x = 1;
	double begin = clock();
	for (int i = 0; i < 50000000; i++) {
		x = sin(x + 0.5);
	}
	double end = clock();
	printf("%.2fsec %f\n", (end - begin) / CLOCKS_PER_SEC, x);
}

static float a[8];

int main()
{
	bench();

	/*
		this generates an optimized code using vinsertf128 without vzeroupper
		under /arch:AVX option .
		Therefore, second bench() is slow.
	*/
	float x = (float)clock();
	for (int i = 0; i < 8; i++) {
		a[i] = x;
	}

	bench();
}
