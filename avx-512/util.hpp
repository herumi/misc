#include <stdio.h>
#include <stdint.h>
#include "../../mcl/src/avx512.hpp"

typedef __attribute__((mode(TI))) unsigned int uint128_t;

uint64_t getMask(size_t n)
{
	if (n == 64) return 0xffffffffffffffff;
	return (uint64_t(1) << n) - 1;
}

template<class T>
void dump(const T& v, const char *msg = 0)
{
	if (msg) printf("%s ", msg);
	const uint64_t *src = (const uint64_t*)&v;
	for (size_t i = 0; i < sizeof(T)/8; i++) {
		printf("%016llx", (long long)src[i]);
		putchar(' ');
	}
	printf("\n");
}

void set(Vec& v, const uint64_t x[8])
{
	memcpy(&v, x, sizeof(v));
}
