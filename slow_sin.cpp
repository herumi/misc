/*
	cl /EHsc /Ox /Ob2 /fp:fast /Zi slow_sin.cpp
	cl /EHsc /Ox /Ob2 /fp:fast /arch:AVX /Feslow_sin_avx.exe /Zi slow_sin.cpp

	Core i7-2600 3.4GHz / Windows 7 Ultimate

	                 VS2012(x64) VS2013(x64)
	slow_sin             1.33       1.34
	slow_sin_avx         1.33       6.27
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

struct A {
	float a[8];
	A()
	{
		const float x = log(2.0f);
		for (int i = 0; i < 8; i++) {
			a[i] = x;
		}
	}
};

const A a;

int main()
{
	double (*f)(double) = sin;
	double begin = clock();
	double x = 1.0;
	double y = 0;
	x = 1;
	y = 0;
	for (int i = 0; i < 100000000; i++) {
		y += f(x);
		x += 1e-6;
	}
	double end = clock();
	printf("x=%f, %.2fsec\n", x, (end - begin) /double(CLOCKS_PER_SEC));
}
