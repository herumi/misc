#include "mcl.h"
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/test.hpp>
#include <mcl/gmp_util.hpp>
#include <gmp.h>
#include <low_func.hpp>
#include <mcl/fp.hpp>

const int C = 1000000;

typedef mcl::FpT<> Fp;

template<int N>
void gmp_mulPre(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	mpn_mul_n((mp_limb_t*)z, (const mp_limb_t*)x, (const mp_limb_t*)y, N);
}

extern "C" void mclb_fp_add8(uint64_t *z, const uint64_t *x, const uint64_t *y, const uint64_t *p);

#define AAA

struct Montgomery {
	typedef mcl::Unit Unit;
	mpz_class p_;
	mpz_class R_; // (1 << (pn_ * 64)) % p
	mpz_class RR_; // (R * R) % p
	Unit rp_; // rp * p = -1 mod M = 1 << 64
	size_t pn_;
	Montgomery() {}
	explicit Montgomery(const mpz_class& p)
	{
		p_ = p;
		rp_ = mcl::bint::getMontgomeryCoeff(mcl::gmp::getUnit(p, 0));
		pn_ = mcl::gmp::getUnitSize(p);
		R_ = 1;
		R_ = (R_ << (pn_ * 64)) % p_;
		RR_ = (R_ * R_) % p_;
	}

	void toMont(mpz_class& x) const { mul(x, x, RR_); }
	void fromMont(mpz_class& x) const { mul(x, x, 1); }

	void mul(mpz_class& z, const mpz_class& x, const mpz_class& y) const
	{
#if 1
		const size_t ySize = mcl::gmp::getUnitSize(y);
		mpz_class c = x * mcl::gmp::getUnit(y, 0);
//z=c&((mpz_class(1) << (64*11))-1); return;
		Unit q = mcl::gmp::getUnit(c, 0) * rp_;
		c += p_ * q;
		c >>= sizeof(Unit) * 8;
		for (size_t i = 1; i < pn_; i++) {
			if (i < ySize) {
				c += x * mcl::gmp::getUnit(y, i);
			}
			Unit q = mcl::gmp::getUnit(c, 0) * rp_;
			c += p_ * q;
			c >>= sizeof(Unit) * 8;
		}
		if (c >= p_) {
			c -= p_;
		}
		z = c;
#else
		z = x * y;
		for (size_t i = 0; i < pn_; i++) {
			Unit q = mcl::gmp::getUnit(z, 0) * rp_;
#ifdef MCL_USE_VINT
			z += p_ * q;
#else
			mpz_class t;
			mcl::gmp::set(t, q);
			z += p_ * t;
#endif
			z >>= sizeof(Unit) * 8;
		}
		if (z >= p_) {
			z -= p_;
		}
#endif
	}
	void mod(mpz_class& z, const mpz_class& xy) const
	{
		z = xy;
		for (size_t i = 0; i < pn_; i++) {
//printf("i=%zd\n", i);
//std::cout << "z=" << std::hex << z << std::endl;
			Unit q = mcl::gmp::getUnit(z, 0) * rp_;
//std::cout << "q=" << q << std::endl;
			mpz_class t;
			mcl::gmp::set(t, q);
			z += p_ * t;
			z >>= sizeof(Unit) * 8;
//std::cout << "z=" << std::hex << z << std::endl;
		}
		if (z >= p_) {
			z -= p_;
		}
//std::cout << "z=" << std::hex << z << std::endl;
	}
};

template<int N>
void mulPreTest()
{
	cybozu::XorShift rg;
	uint64_t x[N], y[N], xy1[N * 2], xy2[N * 2];
	for (int i = 0; i < N; i++) {
		x[i] = rg.get64();
		y[i] = rg.get64();
	}
	mcl_mulPre(xy1, x, y);
	gmp_mulPre<N>(xy2, x, y);
	CYBOZU_TEST_EQUAL_ARRAY(xy1, xy2, N * 2);
	CYBOZU_BENCH_C("mcl_mulPre", C, mcl_mulPre, xy1, x, y);
	CYBOZU_BENCH_C("gmp", C, gmp_mulPre<N>, xy2, x, y);
}

template<int N>
void montTest(const uint64_t *pp, const Montgomery& mont)
{
	uint64_t xa[N], ya[N], xy1a[N], xy2a[N + 1];
#ifdef AAA
	(void)pp;
#endif
	cybozu::XorShift rg;
	for (int i = 0; i < N; i++) {
		xa[i] = rg.get64();
		ya[i] = rg.get64();
	}

	mpz_class x, y, z;
	mcl::gmp::setArray(x, xa, N);
	mcl::gmp::setArray(y, ya, N);

	mont.mul(z, x, y);
	mcl::gmp::getArray(xy1a, N, z);
	const uint64_t dummy = 0x1234567890abc;
	xy2a[N] = dummy;
	mcl_mont(xy2a, xa, ya);
	CYBOZU_TEST_EQUAL_ARRAY(xy1a, xy2a, N);

#ifndef AAA
	mcl::fp::mulMontNFT<N>(xy1a, xa, ya, pp + 1);
	CYBOZU_TEST_EQUAL_ARRAY(xy1a, xy2a, N);
#endif

	for (int i = 0; i < 100; i++) {
		xa[0]++;
		mcl_mont(xy2a, xa, ya);
#ifndef AAA
		mcl::fp::mulMontNFT<N>(xy1a, xa, ya, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(xy1a, xy2a, N);
#endif

		mcl_mont(xy2a, xa, xa);
#ifndef AAA
		mcl::fp::mulMontNFT<N>(xy1a, xa, xa, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(xy1a, xy2a, N);
#endif
	}
	CYBOZU_TEST_EQUAL(xy2a[N], dummy);

	CYBOZU_BENCH_C("mcl_mont", C, mcl_mont, xy1a, xa, ya);
//	CYBOZU_BENCH_C("gmp", C, (mcl::fp::Mont<N, false>::func), xy2a, xa, ya, pp + 1);
}

template<int N>
void modTest(const mpz_class& p, const uint64_t *pp, const Montgomery& mont)
{
#ifdef AAA
	(void)p;
	(void)pp;
#endif
	uint64_t xya[N * 2], z1a[N], z2a[N + 1];
	cybozu::XorShift rg;
	for (int i = 0; i < N * 2; i++) {
		xya[i] = rg.get64();
	}
	xya[N * 2 - 1] &= 0xfffffffffffffful;

	mpz_class xy, z;
	mcl::gmp::setArray(xy, xya, N * 2);

	mont.mod(z, xy);
	mcl::gmp::getArray(z1a, N, z);
	const uint64_t dummy = 0x1234567890abc;
	z2a[N] = dummy;
	mcl_mod(z2a, xya);
#ifndef AAA
	CYBOZU_TEST_EQUAL_ARRAY(z1a, z2a, N);
#endif

#ifndef AAA
	mcl::fp::modRedNFT<N>(z1a, xya, pp + 1);
	CYBOZU_TEST_EQUAL_ARRAY(z1a, z2a, N);
#endif

	for (int i = 0; i < 100; i++) {
		xya[0]++;
		mcl_mod(z2a, xya);
#ifndef AAA
		mcl::fp::modRedNFT<N>(z1a, xya, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(z1a, z2a, N);
#endif
	}
	CYBOZU_TEST_EQUAL(z2a[N], dummy);

	CYBOZU_BENCH_C("mcl_mod", C, mcl_mod, z2a, xya);
//	CYBOZU_BENCH_C("gmp", C, (mcl::fp::MontRed<N, false>::func), z1a, xya, pp + 1);
}

template<int N>
void addTest(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#endif
	uint64_t x1[N], x2[N], y[N];
	cybozu::XorShift rg;
	for (int i = 0; i < N; i++) {
		x1[i] = rg.get64();
		x2[i] = x1[i];
		y[i] = rg.get64();
	}

	for (int i = 0; i < 100; i++) {
		mcl_add(x2, x2, y);
#ifndef AAA
		mcl::fp::addModNFT<N>(x1, x1, y, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N);
#endif
	}
	CYBOZU_BENCH_C("mcl_add", C, mcl_add, x1, x1, x1);
}

template<int N>
void subTest(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#endif
	uint64_t x1[N], x2[N], y[N];
	cybozu::XorShift rg;
	for (int i = 0; i < N; i++) {
		x1[i] = rg.get64();
		x2[i] = x1[i];
		y[i] = rg.get64();
	}

	for (int i = 0; i < 100; i++) {
		mcl_sub(x2, x2, y);
#ifndef AAA
		mcl::fp::subModNFT<N>(x1, x1, y, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N);
#endif
	}
	CYBOZU_BENCH_C("mcl_sub", C, mcl_sub, x1, x1, y);
}

template<int N>
void negTest(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#else
	uint64_t y1[N];
#endif
	uint64_t y2[N], x[N];
	cybozu::XorShift rg;

	for (int i = 0; i < 100; i++) {
		for (int i = 0; i < N; i++) {
			x[i] = rg.get64();
		}
		mcl_neg(y2, x);
#ifndef AAA
		mcl::fp::negT<N>(y1, x, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(y1, y2, N);
#endif
	}
	CYBOZU_BENCH_C("mcl_neg", C, mcl_neg, y2, y2);
	memset(x, 0, sizeof(x));
	mcl_neg(y2, x);
	CYBOZU_TEST_EQUAL_ARRAY(x, y2, N);
}

template<int N>
void mul2Test(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#endif
	uint64_t x1[N], x2[N];
	cybozu::XorShift rg;
	for (int i = 0; i < N; i++) {
		x1[i] = rg.get64();
		x2[i] = x1[i];
	}

	for (int i = 0; i < 100; i++) {
		mcl_mul2(x2, x2);
#ifndef AAA
		mcl::fp::addModNFT<N>(x1, x1, x1, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N);
#endif
	}
	CYBOZU_BENCH_C("mcl_mul2", C, mcl_mul2, x1, x1);
}

template<int N>
void addDblTest(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#endif
	uint64_t x1[N * 2], x2[N * 2], y[N * 2];
	cybozu::XorShift rg;
	for (int i = 0; i < N * 2; i++) {
		x1[i] = rg.get64();
		x2[i] = x1[i];
		y[i] = rg.get64();
	}
	for (int i = 0; i < 100; i++) {
		mcl_addDbl(x2, x2, y);
#ifndef AAA
		mcl::fp::fpDblAddModT<N>(x1, x1, y, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N * 2);
#endif
	}
	CYBOZU_BENCH_C("mcl_addDbl", C, mcl_addDbl, x1, x1, y);
}

template<int N>
void subDblTest(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#endif
	uint64_t x1[N * 2], x2[N * 2], y[N * 2];
	cybozu::XorShift rg;
	for (int i = 0; i < N * 2; i++) {
		x1[i] = rg.get64();
		x2[i] = x1[i];
		y[i] = rg.get64();
	}

	for (int i = 0; i < 100; i++) {
		mcl_subDbl(x2, x2, y);
#ifndef AAA
		mcl::fp::fpDblSubModT<N>(x1, x1, y, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N * 2);
#endif
	}
	CYBOZU_BENCH_C("mcl_subDbl", C, mcl_subDbl, x1, x1, y);
}

template<int N>
void negDblTest(const mpz_class& p)
{
	uint64_t x1[N * 2], x2[N * 2], y[N * 2];

	memset(y, -1, sizeof(y));
	memset(x1, 0, sizeof(x1));
	// neg(0) = 0
	mcl_negDbl(y, x1);
	CYBOZU_TEST_EQUAL_ARRAY(y, x1, N * 2);

	mpz_class pN = p << (N * 64);
	for (int i = 0; i < N * 2; i++) {
		memset(x1, 0, sizeof(x1));
		x1[i] = 0x3;
		mpz_class t;
		mcl::gmp::setArray(t, x1, N * 2);
		if (t != 0) {
			t = pN - t;
		}
		mcl::gmp::getArray(x2, N * 2, t);
		mcl_negDbl(y, x1);
		CYBOZU_TEST_EQUAL_ARRAY(y, x2, N * 2);
	}
}

template<int N>
void mul2DblTest(const uint64_t *pp)
{
#ifdef AAA
	(void)pp;
#endif
	uint64_t x1[N * 2], x2[N * 2];
	cybozu::XorShift rg;
	for (int i = 0; i < N * 2; i++) {
		x1[i] = rg.get64();
		x2[i] = x1[i];
	}
	for (int i = 0; i < 100; i++) {
		mcl_mul2Dbl(x2, x2);
#ifndef AAA
		mcl::fp::addModNFT<N>(x1, x1, x1, pp + 1);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N * 2);
#endif
	}
	CYBOZU_BENCH_C("mcl_mul2Dbl", C, mcl_mul2Dbl, x1, x1);
}

// pTop = p[N - 1]
template<int N>
void addSubPreTest(uint64_t pTop, void3u add, void3u sub, void3u addPre, void3u subPre)
{
	uint64_t x[N], x1[N], x2[N], y[N];
	cybozu::XorShift rg;
	for (int j = 0; j < 100; j++) {
		for (int i = 0; i < N; i++) {
			x[i] = rg.get64();
			y[i] = rg.get64();
			if (i == N - 1) { // x, y < p/2
				x[i] %= pTop/2;
				y[i] %= pTop/2;
			}
		}
		add(x1, x, y);
		addPre(x2, x, y);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N);
		sub(x1, x1, y);
		subPre(x2, x2, y);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x, N);
		CYBOZU_TEST_EQUAL_ARRAY(x2, x, N);
	}
}

template<int N>
void testAll(const char *pStr)
{
	printf("test N=%d\n", N);
	mcl_init(pStr);
	mpz_class p(pStr);
	uint64_t pp[N + 1];
	Montgomery mont(p);
	mcl::gmp::getArray(pp + 1, N, p);
	pp[0] = mont.rp_;
	mulPreTest<N>();
	montTest<N>(pp, mont);
	modTest<N>(p, pp, mont);
	addTest<N>(pp);
	subTest<N>(pp);
	negTest<N>(pp);
	mul2Test<N>(pp);
	addDblTest<N>(pp);
	subDblTest<N>(pp);
	negDblTest<N>(p);
	mul2DblTest<N>(pp);
	addSubPreTest<N>(pp[N], mcl_add, mcl_sub, mcl_addPre, mcl_subPre);
	addSubPreTest<N*2>(pp[N], mcl_addDbl, mcl_subDbl, mcl_addDblPre, mcl_subDblPre);
}

#if 0
CYBOZU_TEST_AUTO(N11)
{
	const char *pStr = "0x9401ff90f28bffb0c610fb10bf9e0fefd59211629a7991563c5e468d43ec9cfe1549fd59c20ab5b9a7cda7f27a0067b8303eeb4b31555cf4f24050ed155555cd7fa7a5f8aaaaaaad47ede1a6aaaaaaaab69e6dcb";
	testAll<11>(pStr);
}

CYBOZU_TEST_AUTO(N9)
{
	const char *pStr = "0xbb9dfd549299f1c803ddd5d7c05e7cc0373d9b1ac15b47aa5aa84626f33e58fe66943943049031ae4ca1d2719b3a84fa363bcd2539a5cd02c6f4b6b645a58c1085e14411";
	testAll<9>(pStr);
}
#endif

CYBOZU_TEST_AUTO(N8)
{
	const char *pStr = "0x65b48e8f740f89bffc8ab0d15e3e4c4ab42d083aedc88c425afbfcc69322c9cda7aac6c567f35507516730cc1f0b4f25c2721bf457aca8351b81b90533c6c87b";
	testAll<8>(pStr);
}

CYBOZU_TEST_AUTO(special)
{
	const size_t N = 8;
	const char *pStr = "0x65b48e8f740f89bffc8ab0d15e3e4c4ab42d083aedc88c425afbfcc69322c9cda7aac6c567f35507516730cc1f0b4f25c2721bf457aca8351b81b90533c6c87b";
	Fp::init(pStr);
	Fp xx, yy;
	mpz_class mp(pStr);
	uint64_t pp[N + 1];
	uint64_t *p = pp + 1;
	Montgomery mont(mp);
	mcl::gmp::getArray(p, N, mp);
	pp[0] = mont.rp_;
	uint64_t x1[N], x2[N], y[N];
	cybozu::XorShift rg;
	for (size_t i = 0; i < N; i++) {
		x1[i] = rg.get64();
		if (i == N - 1) x1[i] &= (uint64_t(1) << 62) - 1;
		x2[i] = x1[i];
		y[i] = rg.get64();
		if (i == N - 1) y[i] &= (uint64_t(1) << 62) - 1;
	}

	for (int i = 0; i < 100; i++) {
		mclb_fp_add8(x2, x2, y, p);
		mcl::fp::addModNFT<N>(x1, x1, y, p);
		CYBOZU_TEST_EQUAL_ARRAY(x1, x2, N);
	}
	xx.setArray(x1, N);
	CYBOZU_BENCH_C("mclb_fp_add8", C, mclb_fp_add8, x1, x1, x1, p);
	CYBOZU_BENCH_C("Fp::add", C, Fp::add,xx, xx, xx);
}
