// for clang+--11
#include <stdio.h>
#include <stdint.h>

typedef unsigned _ExtInt(256) uint256_t;

void dump(const uint256_t& x)
{
	const uint64_t *p = (const uint64_t*)&x;
	for (int i = 0; i < 4; i++) {
		printf("%016lx", p[3 - i]);
	}
	printf("\n");
}

int main()
{
	uint256_t x = 1;
	uint256_t y = 2;
	for (int i = 0; i < 256; i++) {
		dump(x);
		x *= y;
	}
}

