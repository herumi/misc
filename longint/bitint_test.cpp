#include <stdint.h>
#include <stdlib.h>
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
#include <cybozu/test.hpp>
#include <cybozu/xorshift.hpp>
#include <gmpxx.h>

template<class RG>
void setRand(Unit *x, size_t n, RG& rg)
{
	for (size_t i = 0; i < n; i++) {
		x[i] = (Unit)rg.get64();
	}
}

void setArray(mpz_class& z, const Unit *buf, size_t n)
{
	mpz_import(z.get_mpz_t(), n, -1, sizeof(*buf), 0, 0, buf);
}

const size_t C = 10;

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
		CYBOZU_TEST_EQUAL(mz + (mpz_class(CF) << (N * UnitBitSize)), mx + my);
	}
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
			CYBOZU_TEST_EQUAL(mz, mx - my + (mpz_class(CF) << (N * UnitBitSize)));
		}
	}
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
