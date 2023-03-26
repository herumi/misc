#include <stdio.h>
#include <stdint.h>

uint32_t sum(uint32_t n);

int main()
{
	uint32_t v1 = 0;
	for (int i = 0; i < 10; i++) {
		uint32_t v2 = sum(i);
		printf("%d %u %c\n", i, v2, v1 == v2 ? 'o' : 'x');
		v1 += i;
	}
}
