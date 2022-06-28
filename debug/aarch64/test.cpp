#include <stdio.h>
#include <stdint.h>

extern "C" uint32_t func(uint32_t, uint32_t);

int main()
{
	const uint32_t tbl[] = { 0, 1, 0x7fffffff, 0x80000000, 0x80000001, 0xffffffff };
	const size_t n = sizeof(tbl) / sizeof(tbl[0]);
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			uint32_t x = tbl[i];
			uint32_t y = tbl[j];
			uint32_t z = func(x, y);
			printf("%08x+%08x=%08x\n", x, y, z);
		}
	}
}
