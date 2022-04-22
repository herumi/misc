#include "bitint.hpp"
#include <cybozu/test.hpp>
#include <cybozu/xorshift.hpp>

using namespace mcl::vint;
typedef mcl::fp::Unit Unit;

template<class RG>
void setRand(Unit *x, size_t n, RG& rg)
{
	for (size_t i = 0; i < n; i++) {
		x[i] = (Unit)rg.get64();
	}
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
	for (int i = 0; i < 100; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		Unit CF = addT<N>(z, x, y);
		Unit CF2 = subT<N>(x2, z, y);
		CYBOZU_TEST_EQUAL_ARRAY(x, x2, N);
		CYBOZU_TEST_EQUAL(CF, CF2);
	}
}
