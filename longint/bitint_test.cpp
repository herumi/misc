#include <stdint.h>
#include <stdlib.h>
#include <cybozu/inttype.hpp>
typedef uint64_t Unit;
static const size_t UnitBitSize = sizeof(Unit) * 8;
#define MCL_SIZEOF_UNIT 8
// z[N] = x[N] + y[N] and return CF(0 or 1)
template<size_t N>Unit addT(Unit *z, const Unit *x, const Unit *y);
// z[N] = x[N] - y[N] and return CF(0 or 1)
template<size_t N>Unit subT(Unit *z, const Unit *x, const Unit *y);
// [ret:z[N]] = x[N] * y
template<size_t N>Unit mulUnitT(Unit *z, const Unit *x, Unit y);
// [ret:z[N]] = z[N] + x[N] * y
template<size_t N>Unit mulUnitAddT(Unit *z, const Unit *x, Unit y);
#include <mcl/bitint_asm.hpp>
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/link_mpir.hpp>
#include <cybozu/benchmark.hpp>
#include <gmpxx.h>

template<class RG>
void setRand(Unit *x, size_t n, RG& rg)
{
	for (size_t i = 0; i < n; i++) {
		x[i] = (Unit)rg.get64();
	}
}

mpz_class to_mpz(Unit x)
{
	return mp_limb_t(x);
}

void setArray(mpz_class& z, const Unit *buf, size_t n)
{
	mpz_import(z.get_mpz_t(), n, -1, sizeof(*buf), 0, 0, buf);
}

const size_t C = 100;

template<size_t N>
void testAdd()
{
	cybozu::XorShift rg;
	Unit x[N], y[N], z[N], CF;
	mpz_class mx, my, mz;
	for (size_t i = 0; i < C; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		setArray(mx, x, N);
		setArray(my, y, N);
		CF = addT<N>(z, x, y);
		setArray(mz, z, N);
		CYBOZU_TEST_EQUAL(mz + (to_mpz(CF) << (N * UnitBitSize)), mx + my);
	}
	printf("%2zd ", N);
	CYBOZU_BENCH_C("addT", 1000, addT<N>, z, x, y);
}

CYBOZU_TEST_AUTO(add)
{
	testAdd<1>();
	testAdd<2>();
	testAdd<3>();
	testAdd<4>();
	testAdd<5>();
	testAdd<6>();
	testAdd<7>();
	testAdd<8>();
}

template<size_t N>
void testSub()
{
	cybozu::XorShift rg;
	Unit x[N], y[N], z[N], CF;
	mpz_class mx, my, mz;
	for (size_t i = 0; i < C; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		setArray(mx, x, N);
		setArray(my, y, N);
		CF = subT<N>(z, x, y);
		setArray(mz, z, N);
		CYBOZU_TEST_EQUAL(CF, mx < my);
		if (mx >= my) {
			CYBOZU_TEST_EQUAL(mz, mx - my);
		} else {
			CYBOZU_TEST_EQUAL(mz, mx - my + (to_mpz(CF) << (N * UnitBitSize)));
		}
	}
	printf("%2zd ", N);
	CYBOZU_BENCH_C("subT", 1000, subT<N>, z, x, y);
}

CYBOZU_TEST_AUTO(sub)
{
	testSub<1>();
	testSub<2>();
	testSub<3>();
	testSub<4>();
	testSub<5>();
	testSub<6>();
	testSub<7>();
	testSub<8>();
}

template<size_t N>
void testMulUnit()
{
	cybozu::XorShift rg;
	Unit x[N], y, z[N], ret;
	mpz_class mx, mz;
	for (size_t i = 0; i < C; i++) {
		setRand(x, N, rg);
		setRand(&y, 1, rg);
		setArray(mx, x, N);
		ret = mulUnitT<N>(z, x, y);
		setArray(mz, z, N);
		CYBOZU_TEST_EQUAL(mz + (to_mpz(ret) << (N * UnitBitSize)), mx * to_mpz(y));
	}
	printf("%2zd ", N);
	CYBOZU_BENCH_C("mulUnitT", 1000, mulUnitT<N>, z, x, y);
}

CYBOZU_TEST_AUTO(mulUnit)
{
	testMulUnit<1>();
	testMulUnit<2>();
	testMulUnit<3>();
	testMulUnit<4>();
	testMulUnit<5>();
	testMulUnit<6>();
	testMulUnit<7>();
	testMulUnit<8>();
}

template<size_t N>
void testMulUnitAdd()
{
	cybozu::XorShift rg;
	Unit x[N], y, z[N], ret;
	mpz_class mx, mz, mz2;
	for (size_t i = 0; i < C; i++) {
		setRand(x, N, rg);
		setRand(z, N, rg);
		setRand(&y, 1, rg);
		setArray(mx, x, N);
		setArray(mz, z, N);
		ret = mulUnitAddT<N>(z, x, y);
		setArray(mz2, z, N);
		CYBOZU_TEST_EQUAL(mz2 + (to_mpz(ret) << (N * UnitBitSize)), mz + mx * to_mpz(y));
	}
	printf("%2zd ", N);
	CYBOZU_BENCH_C("mulUnitAddT", 1000, mulUnitAddT<N>, z, x, y);
}

CYBOZU_TEST_AUTO(mulUnitAdd)
{
	testMulUnitAdd<1>();
	testMulUnitAdd<2>();
	testMulUnitAdd<3>();
	testMulUnitAdd<4>();
	testMulUnitAdd<5>();
	testMulUnitAdd<6>();
	testMulUnitAdd<7>();
	testMulUnitAdd<8>();
}

int main(int argc, char *argv[])
{
	bool fast = argc == 2;
	printf("fast=%d\n", fast);
	if (fast) mclb_enable_fast();
	return cybozu::test::autoRun.run(argc, argv);
}

