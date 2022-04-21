#include "bitint.hpp"
#include <cybozu/test.hpp>

using namespace mcl::vint;
typedef mcl::fp::Unit Unit;

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

