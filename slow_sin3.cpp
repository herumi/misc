/*
	fast
	cl /EHsc /Ox /fp:fast slow_sin3.cpp

	second bench is slow
	cl /EHsc /Ox /fp:fast /arch:AVX slow_sin3.cpp
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef USE_XBYAK
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#endif

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

#ifdef USE_XBYAK
const struct Code : Xbyak::CodeGenerator {
	Code()
	{
		vzeroupper();
		ret();
	}
} code;

void (*call_vzeroupper)() = code.getCode<void (*)()>();
#endif

int main(int argc, char *[])
{
	bench();

	/*
		this generates an optimized code using vinsertf128 without vzeroupper
		under /arch:AVX option .
		Therefore, second bench() is slow.
	*/
	float x = (float)argc;
	for (int i = 0; i < 8; i++) {
		a[i] = x;
	}
#ifdef USE_XBYAK
	if (argc == 1) {
		puts("not call vzeroupper");
	} else {
		puts("call vzeroupper");
		call_vzeroupper();
	}
#endif

	bench();
}
