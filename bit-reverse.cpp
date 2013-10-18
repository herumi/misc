#include <stdio.h>
#include <stdint.h>

#ifdef _MSC_VER
	#include <intrin.h>
	#define MIE_ALIGN(x) __declspec(align(x))
#else
	#include <x86intrin.h>
	#define MIE_ALIGN(x) __attribute__((aligned(x)))
#endif

uint32_t bitReverse(uint32_t x, size_t n)
{
	uint32_t y = 0;
	for (size_t i = 0; i < n; i++) {
		y <<= 1;
		if (x & 1) y++;
		x >>= 1;
	}
	return y;
}

void put(uint32_t a, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		putchar(((a >> (n - 1 - i)) & 1) + '0');
	}
	printf(" %u\n", a);
}

template<int n>
struct Rev {};

template<>
struct Rev<8> {
	enum { len = 8 };
	__m128i step_;
	__m128i x_;
	Rev()
	{
		const MIE_ALIGN(16) uint8_t step[16] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
		step_ = *(const __m128i*)step;
		x_ = _mm_setzero_si128();
	}
	uint32_t next()
	{
		uint32_t a = _mm_movemask_epi8(x_);
		x_ = _mm_add_epi8(x_, step_);
		return a;
	}
};

template<>
struct Rev<9> {
	enum { len = 9 };
	__m128i step_;
	__m128i one_;
	__m128i x_;
	Rev()
	{
		const MIE_ALIGN(16) uint8_t step[16] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
		const MIE_ALIGN(16) uint8_t one[16] = { 1 };
		step_ = *(const __m128i*)step;
		one_ = *(const __m128i*)one;
		x_ = _mm_setzero_si128();
	}
	uint32_t next()
	{
		uint32_t a = _mm_movemask_epi8(x_);
		x_ = _mm_add_epi8(x_, step_);
		step_ = _mm_xor_si128(step_, one_);
		return a;
	}
};

template<class Rev>
void loop()
{
	const int len = Rev::len;
	const int n = 1 << len;
	printf("loop%d\n", n);
	Rev rev;
	for (int i = 0; i < n; i++) {
		uint32_t r1 = rev.next();
		uint32_t r2 = bitReverse(i, len);
		if (r1 != r2) {
			puts("ERR");
			printf("i=%d\n", i);
			put(r1, len);
			put(r2, len);
			return;
		}
	}
	puts("ok");
}

int main()
{
	loop<Rev<8> >();
	loop<Rev<9> >();
}
