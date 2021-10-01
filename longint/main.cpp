#include "mcl.h"
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/test.hpp>
#include <gmp.h>

static const int N = 11;

void gmp_mulPre(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	mpn_mul_n((mp_limb_t*)z, (const mp_limb_t*)x, (const mp_limb_t*)y, N);
}

CYBOZU_TEST_AUTO(mulPre)
{
	mcl_init();
	cybozu::XorShift rg;
	uint64_t x[N], y[N], xy1[N * 2], xy2[N * 2];
	for (int i = 0; i < N; i++) {
		x[i] = rg.get64();
		y[i] = rg.get64();
	}
	puts("a");
	mcl_mulPre(xy1, x, y);
	gmp_mulPre(xy2, x, y);
	CYBOZU_TEST_EQUAL_ARRAY(xy1, xy2, N * 2);
	const int C = 100000;
	CYBOZU_BENCH_C("mcl", C, mcl_mulPre, xy1, x, y);
	CYBOZU_BENCH_C("gmp", C, gmp_mulPre, xy2, x, y);
}
