#include <stdio.h>
#include <assert.h>

int calc(int a, int b, int s)
{
	const int Q = 1 << s, Q2 = Q * 2, Q3 = Q * 3;
	assert(s <= 16 && b < a && a < Q * 4);
	int n = 0;
	for (;;) {
		if (a < Q2) {
			n = n * 2;
		} else if (b >= Q2) {
			n = n * 2 + 1;
			b -= Q2;
			a -= Q2;
		} else if (b >= Q && a < Q3) {
			b -= Q;
			a -= Q;
		} else {
			break;
		}
		b = b * 2;
		a = a * 2 + 1;
	}
	return n;
}

int main()
{
	int s = 3;
	for (int a = 1; a < 32; a++) {
		printf("a=%2d ", a);
		for (int b = 0; b < a; b++) {
			printf("%2d ", calc(a, b, s));
		}
		printf("\n");
	}
}
