#include <iostream>
#include <cybozu/benchmark.hpp>
#include "montgomery.hpp"

void test(const mpz_class& p)
{
	std::cout << "p=0x" << std::hex << p << std::endl;
	Montgomery mont(p);

	mpz_class x1("123456789012345678");
	mpz_class y1("987654321092342342");
	for (int i = 0; i < 100; i++) {
		mpz_class z1 = (x1 * y1) % p;
		mpz_class x2, y2, z2;
		mont.toMont(x2, x1);
		mont.toMont(y2, y1);
		mont.mul(z2, x2, y2);
		mont.fromMont(z2, z2);
		if (z1 != z2) {
			puts("ERR");
			std::cout << z1 << std::endl;
			std::cout << z2 << std::endl;
			return;
		}
		x1 = z1;
	}
}

int main()
{
	const char *pTbl[] = {
		"0x2523648240000001ba344d80000000086121000000000013a700000000000013",
		"0xfffffffffffffffffffffffffffffffffffffffeffffee37",
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(pTbl); i++) {
		const mpz_class p(pTbl[i]);
		test(p);
	}
}
