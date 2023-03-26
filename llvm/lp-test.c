#include <stdio.h>
#include <stdint.h>

uint32_t sum(uint32_t n);

int main()
{
	for (int i = 0; i < 10; i++) {
		printf("%d %d\n", i, sum(i));
	}
}
