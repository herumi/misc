#include <stdio.h>
#include <stdint.h>

static uint64_t ullog2(uint64_t x)
{
    static const uint64_t debruijn_magic = 0x022fdd63cc95386dULL;

    static const uint64_t magic_table[] = {
        0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28,
        62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
        63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
        51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12,
    };

    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return (magic_table[((x & ~(x>>1))*debruijn_magic)>>58]);
}

int ilog2(uint64_t x)
{
	return 63 - __builtin_clzll(x);
}

void check(uint64_t x)
{
	int a = ullog2(x);
	int b = ilog2(x);
	if (a != b) {
		printf("x=%lld a=%d b=%d\n", (long long)x, a, b);
	}
}

int main()
{
	for (int x = 0; x < 100000; x++) {
		check(x);
	}
	for (int i = 1; i < 64; i++) {
		uint64_t x = (uint64_t)1 << i;
		check(x - 1);
		check(x);
		check(x + 1);
	}
}

