#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _MSC_VER

#pragma warning(disable : 4146)
#include <intrin.h>
uint64_t bsr(uint64_t x)
{
	unsigned long out;
	_BitScanReverse64(&out, x);
	return out;
}

#else

#include <x86intrin.h>
uint64_t bsr(uint64_t x)
{
	return __builtin_clzl(x) ^ 0x3f;
}

#endif

uint64_t nextCombination(uint64_t a)
{
	if (a & 1) {
		uint64_t b = a ^ (a + 1);
		uint64_t c = a - b / 2;
#if 1
		return c - ((c & -c) >> bsr(b + 1));
#else
		return c - (c & -c) / (b + 1);
#endif
	} else {
		return a - (a & -a) / 2;
	}
}

void printB(uint64_t n, uint64_t a)
{
	for (uint64_t i = 0; i < n; i++) {
		printf("%c ", 1 & (a >> (n - 1 - i)) ? '1' : '0');
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	argc--, argv++;
	if (argc != 2) {
		printf("nCk n k\n");
		return 1;
	}
	const uint32_t n = atoi(argv[0]);
	const uint32_t k = atoi(argv[1]);
	if (n >= 64 || k > n) {
		printf("bad n=%u, k=%u\n", n, k);
		return 1;
	}
	/* 11...100....00
	 *       <- k  ->
	 * <---  n  ---->
	 */
	uint64_t a = ((uint64_t(1) << (n - k)) - 1) << k;
	while (a) {
		printB(n, a);
		a = nextCombination(a);
	}
}
