#include <stdio.h>
#include <stdint.h>
#include <memory.h>

int main()
{
	typedef __attribute__((mode(TI))) unsigned int uint128;
	uint128 a = 1;
	for (int i = 0; i < 128; i++) {
		uint64_t b[2];
		memcpy(b, &a, sizeof(b));
		printf("%016lx %016lx\n", b[1], b[0]);
		a <<= 1;
	}
}
