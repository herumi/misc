#include <cybozu/test.hpp>

#include "gf256.hpp"
#include "mul.h"

extern "C" {

void gf256_mul_gfni(uint8_t *pz, const uint8_t *px, const uint8_t *py);
void gf256_inv_gfni(uint8_t *py, const uint8_t *px);

}

static uint8_t g_mulTbl[256 * 256];

uint8_t mulTbl(uint8_t x, uint8_t y)
{
	return g_mulTbl[x + y * 256];
}

CYBOZU_TEST_AUTO(mulTbl)
{
	for (uint32_t x = 0; x < 256; x++) {
		for (uint32_t y = 0; y < 256; y++) {
			g_mulTbl[x + y * 256] = gf256_mul(x, y);
		}
	}
}

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
			CYBOZU_TEST_EQUAL(z1, mulTbl(x, y));
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

CYBOZU_TEST_AUTO(mulVec)
{
	const size_t N = 32;
	uint8_t pz[N], px[N], py[N];
	for (uint32_t x = 0; x < 256; x++) {
		for (size_t i = 0; i < N; i++) {
			px[i] = uint8_t(x + i);
		}
		for (uint32_t y = 0; y < 256; y++) {
			for (size_t i = 0; i < N; i++) {
				py[i] = uint8_t(y + i);
			}
			gf256_mul_gfni(pz, px, py);
			for (size_t i = 0; i < N; i++) {
				CYBOZU_TEST_EQUAL(pz[i], gf256_mul(px[i], py[i]));
			}
		}
	}
}

CYBOZU_TEST_AUTO(invVec)
{
	const size_t N = 32;
	uint8_t py[N], px[N];
	for (uint32_t x = 0; x < 256; x++) {
		for (size_t i = 0; i < N; i++) {
			px[i] = uint8_t(x + i);
		}
		gf256_inv_gfni(py, px);
#if 0
		puts("ok");
		for (size_t i = 0; i < N; i++) printf("%02x ", gf256_inv(px[i]));
		puts("");
		puts("my");
		for (size_t i = 0; i < N; i++) printf("%02x ", py[i]);
		puts("");
#else
		for (size_t i = 0; i < N; i++) {
			CYBOZU_TEST_EQUAL(py[i], gf256_inv(px[i]));
		}
#endif
	}
}

#if 0
CYBOZU_TEST_AUTO(divVec)
{
	const size_t N = 32;
	uint8_t pz[N], px[N], py[N];
	for (uint32_t x = 0; x < 256; x++) {
		for (size_t i = 0; i < N; i++) {
			px[i] = uint8_t(x + i);
		}
		for (uint32_t y = 0; y < 256; y++) {
			for (size_t i = 0; i < N; i++) {
				py[i] = uint8_t(y + i);
			}
			gf256_div_gfni(pz, px, py);
			for (size_t i = 0; i < N; i++) {
				CYBOZU_TEST_EQUAL(pz[i], gf256_div(px[i], py[i]));
			}
		}
	}
}
#endif

