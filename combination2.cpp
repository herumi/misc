#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// return the next bit pattern of n-bits which contains k zeros.
uint64_t nextCombination(uint64_t a)
{
	uint64_t b = a ^ (a + 1);
	uint64_t c = a - b / 2;
	return c - (c & -c) / (b + 1);
}

void putB(uint64_t n, uint64_t a)
{
	for (uint64_t i = 0; i < n; i++) {
		printf("%c ", 1 & (a >> (n - 1 - i)) ? '1' : '0');
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "conv n k\n");
		return 1;
	}
	const int n = atoi(argv[1]);
	const int k = atoi(argv[2]);
	uint64_t a = ((uint64_t(1) << (n - k)) - 1) << k;
	int c = 1;
	do {
		printf("% 8d ", c++);
		putB(n, a);
	} while (a = nextCombination(a));
}
