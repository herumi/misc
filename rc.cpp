#include <stdio.h>
#include <memory.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

void put(__m128 x)
{
	float f[4];
	memcpy(f, &x, sizeof(f));
	printf("%5g %5g %5g %5g\n", f[0], f[1], f[2], f[3]);
}

int main()
{
	const float a[] = { 1.1f, 1.8f, -2.2f, -2.8f };
	__m128 x = _mm_loadu_ps(a);
	printf("     ");
	put(x);
	for (int i = 0; i < 4; i++) {
		const int rcMask = 3 << 13;
		int cw = _mm_getcsr();
		cw &= ~rcMask;
		cw |= i << 13;
		printf("%04x ", cw);
		__m128i y = _mm_cvtps_epi32(x);
		_mm_setcsr(cw);
		put(_mm_cvtepi32_ps(y));
	}
}

