#include "gf256.hpp"
#include "mul.h"

#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#define XBYAK_ONLY_CLASS_CPU
#include <xbyak/xbyak_util.h>

uint64_t bitRevC(uint64_t x)
{
	uint64_t y = x & 0x0101010101010101ull;
	for (int i = 0; i < 7; i++) {
		y <<= 1;
		x >>= 1;
		y |= x & 0x0101010101010101ull;
	}
	uint64_t z = y & 0xff;
	for (int i = 0; i < 7; i++) {
		z <<= 8;
		y >>= 8;
		z |= y & 0xff;
	}
	return z;
}

extern "C" {

#ifdef XBYAK_INTEL_CPU_SPECIFIC
void gf256_mul_gfni(uint8_t *pz, const uint8_t *px, const uint8_t *py);
void gf256_inv_gfni(uint8_t *py, const uint8_t *px);
void gf256_mul_gfni512(uint8_t *pz, const uint8_t *px, const uint8_t *py);
void gf256_inv_gfni512(uint8_t *py, const uint8_t *px);
void a_pclmulqdq(uint8_t pz[16], const uint8_t px[8], const uint8_t py[8]);
uint64_t bitRev_gfni(uint64_t x);
#else
void gf256_mul_gfni(uint8_t *pz, const uint8_t *px, const uint8_t *py)
{
	(void)pz;
	(void)px;
	(void)py;
}
void gf256_inv_gfni(uint8_t *py, const uint8_t *px)
{
	(void)py;
	(void)px;
}
void gf256_mul_gfni512(uint8_t *pz, const uint8_t *px, const uint8_t *py)
{
	(void)pz;
	(void)px;
	(void)py;
}
void gf256_inv_gfni512(uint8_t *py, const uint8_t *px)
{
	(void)py;
	(void)px;
}
void a_pclmulqdq(uint8_t pz[16], const uint8_t px[8], const uint8_t py[8])
{
	(void)pz;
	(void)px;
	(void)py;
}
uint64_t bitRev_gfni(uin64_t x)
{
	return bitRevC(x);
}
#endif

}

static Xbyak::util::Cpu cpu;
bool hasGFNI() { return cpu.has(Xbyak::util::Cpu::tGFNI); }
bool hasGFNI512() { return cpu.has(Xbyak::util::Cpu::tGFNI|Xbyak::util::Cpu::tAVX512F); }

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

#ifdef XBYAK_INTEL_CPU_SPECIFIC
CYBOZU_TEST_AUTO(pclmulqdq)
{
	for (uint32_t x = 0; x < 256; x++) {
		for (uint32_t y = 0; y < 256; y++) {
			uint32_t z1 = mulPoly(x, y);
			uint8_t px[8]={};
			uint8_t py[8]={};
			uint8_t pz[16]={};
			px[0] = x;
			py[0] = y;
			a_pclmulqdq(pz, px, py);
			uint32_t z2 = pz[0] + pz[1] * 256;
			CYBOZU_TEST_EQUAL(z1, z2);
		}
	}
}
#endif

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
			CYBOZU_TEST_EQUAL(z1, modPoly(mulPoly(x, y)));
		}
	}
	{
		uint8_t x = 0x12;
		const int C = 10000;
		CYBOZU_BENCH_C("gf256_mul", C, x = gf256_mul, x, x+1);
		printf("x=%02x\n", x);
		CYBOZU_BENCH_C("mulTbl", C, x = mulTbl, x, x+1);
		printf("x=%02x\n", x);
		CYBOZU_BENCH_C("mul.h", C, x = mul, x, x+1);
		printf("x=%02x\n", x);
		CYBOZU_BENCH_C("mul+mod", C, x = mulMod, x, x+1);
		printf("x=%02x\n", x);
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
	if (!hasGFNI()) { puts("skip"); return; }
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
	const int C = 10000;
	CYBOZU_BENCH_C("gf256_mul_gfni", C, gf256_mul_gfni, px, px, px);
}

CYBOZU_TEST_AUTO(mulVec512)
{
	if (!hasGFNI512()) { puts("skip"); return; }
	const size_t N = 64;
	uint8_t pz[N], px[N], py[N];
	for (uint32_t x = 0; x < 256; x++) {
		for (size_t i = 0; i < N; i++) {
			px[i] = uint8_t(x + i);
		}
		for (uint32_t y = 0; y < 256; y++) {
			for (size_t i = 0; i < N; i++) {
				py[i] = uint8_t(y + i);
			}
			gf256_mul_gfni512(pz, px, py);
			for (size_t i = 0; i < N; i++) {
				CYBOZU_TEST_EQUAL(pz[i], gf256_mul(px[i], py[i]));
			}
		}
	}
	const int C = 10000;
	CYBOZU_BENCH_C("gf256_mul_gfni512", C, gf256_mul_gfni512, px, px, px);
}

CYBOZU_TEST_AUTO(invVec)
{
	if (!hasGFNI()) { puts("skip"); return; }
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

CYBOZU_TEST_AUTO(bitRev)
{
	if (!hasGFNI()) { puts("skip"); return; }
	cybozu::XorShift rg;
	for (int i = 0; i < 500; i++) {
		uint64_t x = i;//rg.get64();
		uint64_t y = bitRevC(x);
		CYBOZU_TEST_EQUAL(x, bitRevC(bitRevC(x)));
		uint64_t z = bitRev_gfni(x);
//		printf("x=%016llx y=%016llx z=%016llx\n", (long long)x, (long long)y, (long long)z);
		CYBOZU_TEST_EQUAL(y, z);
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

