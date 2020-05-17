/*
	g++ -O3 -mavx512f -mavx512vl -mavx512bw mask-load.cpp
*/
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif

__m128i load(const void *p, size_t n)
{
#if !defined(__AVX512BW__) || !defined(__AVX512VL__)
	#error "-mavx512bw -mavx512vl"
#endif
	if (n == 16) {
		return _mm_loadu_si128((const __m128i*)p);
	} else {
		__mmask16 k =  _mm512_kmov((1 << n) - 1);
		return _mm_maskz_loadu_epi8(k, p);
	}
}

int main()
{
	const int N = 16;
	char buf[N];
	for (int i = 0; i < N; i++) {
		buf[i] = char(i + 1);
	}
	for (int i = 0; i < N + 1; i++) {
		__m128i x = load(buf, i);
		printf("%2d ", i);
		unsigned char b[N];
		memcpy(b, &x, N);
		for (int j = 0; j < N; j++) {
			printf("%02x", b[j]);
		}
		printf("\n");
	}
}
