// cbmc --unwind 20 --trace --function squareRoot squre.c
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#ifdef CBMC
  #define CBMC_ASSUME __CPROVER_assume
#else
  #define CBMC_ASSUME(x) ((void)0)
#endif

uint8_t squareRoot(uint16_t x)
{
	uint16_t s = 0;
	uint16_t e = 255; // 255^2 = 65025 < 65535
	while (s <= e) {
		uint16_t m = (s + e) / 2;
		uint32_t m2 = (uint32_t)m * m;
		if (m2 <= x && x < m2 + 2 * m + 1) {
			CBMC_ASSUME(m * m <= x && x < (m + 1) * (m + 1));
			return (uint8_t)m;
		} else if (m2 < x) {
			s = m + 1;
		} else {
			e = m - 1;
		}
	}
	printf("x=%d s=%d e=%d\n", x, s, e);
	assert(0);
}

/*
int main()
{
	uint16_t x = 29931;
	uint8_t r = squareRoot(x);
	printf("r=%d r * r=%d\n", r, r * r);
}
*/