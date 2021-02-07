#include <stdio.h>
#include <stdint.h>

inline uint64_t mulUnit(uint64_t *pH, uint64_t x, uint64_t y)
{
	const uint64_t mask = 0xffffffff;
	uint64_t v = (x & mask) * (y & mask);
	uint64_t L = uint32_t(v);
	uint64_t H = v >> 32;
	uint64_t ad = (x & mask) * uint32_t(y >> 32);
	uint64_t bc = uint32_t(x >> 32) * (y & mask);
	H += uint32_t(ad);
	H += uint32_t(bc);
	L |= H << 32;
	H >>= 32;
	H += ad >> 32;
	H += bc >> 32;
	H += (x >> 32) * (y >> 32);
	*pH = H;
	return L;
}

typedef __attribute__((mode(TI))) unsigned int uint128;

int main()
{
	uint64_t x = 0x1234567890123;
	uint64_t y = 0x23429834723487;
	uint64_t L, H;
	L = mulUnit(&H, x, y);
	printf("%016lx:%016lx\n", H, L);
	uint128 z = uint128(x) * uint128(y);
	printf("%016lx:%016lx\n", uint64_t(z >> 64), uint64_t(z));
}

