#include <cybozu/test.hpp>
#include <cybozu/xorshift.hpp>
#include <mcl/gmp_util.hpp>
#include "mul.hpp"

template<class RG>
void setRand(uint32_t *x, size_t n, RG& rg)
{
	for (size_t i = 0; i < n; i++) {
		x[i] = rg.get32();
	}
}

CYBOZU_TEST_AUTO(mul2)
{
	const size_t N = 2;
	mpz_class mz, mx, my;
	uint32_t x[N], y[N], z[N * 2];
	cybozu::XorShift rg;
	std::cout << std::hex;
	for (int i = 0; i < 1; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		mcl::gmp::setArray(mx, x, N);
		mcl::gmp::setArray(my, y, N);
		mz = mx * my;
		mulT2(z, x, y);
		uint32_t ok[N * 2];
		mcl::gmp::getArray(ok, N * 2, mz);
		CYBOZU_TEST_EQUAL_ARRAY(z, ok, N * 2);
	}
}
