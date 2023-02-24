#pragma once

#include <stdint.h>

inline uint8_t mul(uint8_t x, uint8_t y)
{
	uint8_t ret = 0;
	while (x && y) {
		if (y & 1) ret ^= x;
		if (x & 0x80) {
			x = (x << 1) ^ 0x11b;
		}else{
			x <<= 1;
		}
		y >>= 1;
	}
	return ret;
}

