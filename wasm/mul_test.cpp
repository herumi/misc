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

template<size_t N>
void testMul()
{
	printf("testMul%zd\n", N);
	mpz_class mz, mx, my;
	uint32_t x[N], y[N], z[N * 2];
	cybozu::XorShift rg;
	std::cout << std::hex;
	for (int i = 0; i < 10; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		mcl::gmp::setArray(mx, x, N);
		mcl::gmp::setArray(my, y, N);
		mz = mx * my;
		mulT<N>(z, x, y);
		uint32_t ok[N * 2];
		mcl::gmp::getArray(ok, N * 2, mz);
		CYBOZU_TEST_EQUAL_ARRAY(z, ok, N * 2);
	}
}

CYBOZU_TEST_AUTO(mulTest)
{
	testMul<1>();
	testMul<2>();
	testMul<3>();
}

