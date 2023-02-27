#include <cybozu/test.hpp>

#include "gf256.hpp"
#include "mul.h"

CYBOZU_TEST_AUTO(inv)
{
	for (uint32_t x = 1; x < 256; x++) {
		uint8_t y = gf256_inv(x);
		uint8_t xy = mul(x, y);
		CYBOZU_TEST_EQUAL(xy, 1);
	}
}

CYBOZU_TEST_AUTO(mul)
{
	for (uint32_t x = 0; x < 256; x++) {
		for (uint32_t y = 0; y < 256; y++) {
			uint8_t z1 = mul(x, y);
			uint8_t z2 = gf256_mul(x, y);
			CYBOZU_TEST_EQUAL(z1, z2);
		}
	}
}

CYBOZU_TEST_AUTO(div)
{
	for (uint32_t x = 0; x < 256; x++) {
		for (uint32_t y = 1; y < 256; y++) {
			uint8_t z1 = mul(x, gf256_inv(y));
			uint8_t z2 = gf256_div(x, y);
			CYBOZU_TEST_EQUAL(z1, z2);
		}
	}
}
