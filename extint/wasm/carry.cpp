#include <stdint.h>

bool add(uint64_t *z, uint64_t x, uint64_t y)
{
	uint64_t t = x + y;
	*z = t;
	return t < x;
}

uint64_t adc64(uint64_t *z, uint64_t x, uint64_t y, uint64_t c)
{
	uint64_t xc = x + c;
	if (xc < c) {
		*z = y;
	} else {
		xc += y;
		c = xc < y;
		*z = xc;
	}
	return xc;
}

uint32_t adc32(uint32_t *z, uint32_t x, uint32_t y, uint32_t c)
{
	uint64_t t = uint64_t(x) + y + c;
	*z = uint32_t(t);
	return t >> 32;
}
