/*
	cl /EHsc /Ox /Ob2 /fp:fast /arch:AVX /Zi slow_sin2.cpp
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <cybozu/benchmark.hpp>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

const struct Code : Xbyak::CodeGenerator {
	Code()
	{
		vinsertf128(ym0, ym0, xm0, 1);
//		vzeroupper();
		ret();
	}
} code;

void (*call_vinsertf128)() = code.getCode<void (*)()>();

int main()
{
	double (*f)(double) = sin;
	printf("f=%p\n", f);
	double x = 1.0;
	CYBOZU_BENCH("before vinsertf128", x = f, x);
	printf("x=%f\n", x);

#if 1
	call_vinsertf128();
#else
	{
		puts("a");
		__m128d x;// = _mm_setzero_si128();
		__m256d y;// = _mm256_setzero_si256();
		y = _mm256_insertf128_pd(y, x, 1);
		printf("%d\n", *(int*)&y);
	}
#endif
	CYBOZU_BENCH("after vinsertf128", x = f, x);
	printf("x=%f\n", x);
}
