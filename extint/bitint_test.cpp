#include "bitint.hpp"
#include <cybozu/test.hpp>
#include <cybozu/xorshift.hpp>
#include <gmpxx.h>

using namespace mcl::vint;
typedef mcl::fp::Unit Unit;

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

CYBOZU_TEST_AUTO(cmpT)
{
	const Unit x[] = { 1, 2, 4, 5 };
	const Unit y[] = { 1, 2, 3, 8 };

	CYBOZU_TEST_ASSERT(!cmpEqT<3>(x, y));
	CYBOZU_TEST_ASSERT(cmpEqT<3>(x, x));
	CYBOZU_TEST_ASSERT(cmpEqT<2>(x, y));

	CYBOZU_TEST_ASSERT(cmpGeT<3>(x, y));
	CYBOZU_TEST_ASSERT(cmpGeT<3>(x, x));
	CYBOZU_TEST_ASSERT(!cmpGeT<3>(y, x));
	CYBOZU_TEST_ASSERT(cmpGeT<2>(x, y));
	CYBOZU_TEST_ASSERT(!cmpGeT<4>(x, y));

	CYBOZU_TEST_ASSERT(cmpGtT<3>(x, y));
	CYBOZU_TEST_ASSERT(!cmpGtT<3>(x, x));
	CYBOZU_TEST_ASSERT(!cmpGtT<3>(y, x));

	CYBOZU_TEST_ASSERT(!cmpLeT<3>(x, y));
	CYBOZU_TEST_ASSERT(cmpLeT<3>(x, x));
	CYBOZU_TEST_ASSERT(cmpLeT<3>(y, x));
	CYBOZU_TEST_ASSERT(cmpLeT<2>(x, x));
	CYBOZU_TEST_ASSERT(cmpLeT<4>(x, y));
}

CYBOZU_TEST_AUTO(addT)
{
	const Unit x[] = { 9, 2, 3 };
	const Unit y[] = { Unit(-1), 5, 4 };
	const Unit ok[] = { 8, 8, 7 };
	Unit z[3], CF;
	z[0] = 9;
	z[1] = 10;
	CF = addT<1>(z, x, y);
	CYBOZU_TEST_EQUAL(z[0], ok[0]);
	CYBOZU_TEST_EQUAL(CF, 1);
	CYBOZU_TEST_EQUAL(z[1], 10); // not changed

	CF = addT<1>(z, x + 1, y + 1);
	CYBOZU_TEST_EQUAL(z[0], x[1] + y[1]);
	CYBOZU_TEST_EQUAL(CF, 0);
	CYBOZU_TEST_EQUAL(z[1], 10); // not changed

	CF = addT<3>(z, x, y);
	CYBOZU_TEST_EQUAL_ARRAY(z, ok, 3);
	CYBOZU_TEST_EQUAL(CF, 0);
}

CYBOZU_TEST_AUTO(subT)
{
	const size_t N = 4;
	Unit x[N], y[N], z[N], x2[N];
	cybozu::XorShift rg;
	mpz_class mx, my, mz;
	for (int i = 0; i < 100; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		Unit CF = addT<N>(z, x, y);
		setArray(mx, x, N);
		setArray(my, y, N);
		setArray(mz, z, N);
		CYBOZU_TEST_EQUAL(mx + my, mz + (mpz_class(CF) << (sizeof(x) * 8)));
		Unit CF2 = subT<N>(x2, z, y);
		CYBOZU_TEST_EQUAL_ARRAY(x, x2, N);
		CYBOZU_TEST_EQUAL(CF, CF2);

	}
}

CYBOZU_TEST_AUTO(mulUnitT)
{
	const size_t N = 4;
	Unit x[N], z[N];
	cybozu::XorShift rg;
	mpz_class mx, mz;
	for (int i = 0; i < 100; i++) {
		Unit y;
		setRand(x, N, rg);
		setRand(&y, 1, rg);
		Unit u = mulUnitT<N>(z, x, y);
		setArray(mx, x, N);
		setArray(mz, z, N);
		CYBOZU_TEST_EQUAL(mx * y, mz + (mpz_class(u) << (sizeof(x) * 8)));
	}
}

CYBOZU_TEST_AUTO(mulUnitAddT)
{
	const size_t N = 4;
	Unit x[N], y[N], w[N];
	cybozu::XorShift rg;
	mpz_class mx, my, mw;
	for (int i = 0; i < 100; i++) {
		Unit z;
		setRand(x, N, rg);
		setRand(y, N, rg);
		setRand(&z, 1, rg);
		Unit u = mulUnitAddT<N>(w, x, y, z);
		setArray(mx, x, N);
		setArray(my, y, N);
		setArray(mw, w, N);
		CYBOZU_TEST_EQUAL(mx + my * z, mw + (mpz_class(u) << (sizeof(x) * 8)));
	}
}