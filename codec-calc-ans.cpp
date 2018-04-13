#include <stdio.h>
#include <assert.h>

int calc(int a, int b, int s)
{
	const int Q = 1 << s, Q2 = Q * 2, Q3 = Q * 3;
	assert(0 <= s && s <= 16 && 0 <= b && b < a && a < Q * 4);
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

static inline int bsr(int x)
{
	assert(x > 0);
	union {
		int i;
		float f;
	} ui;
	ui.f = (float)x;
	return (ui.i >> 23) - 127; /* equal to "bsr eax, x" */
}

int calc2(int a, int b, int)
{
	return a >> (bsr(a ^ b) + 1);
//	return b >> (bsr(a ^ b) + 1); // same
}

int main()
{
	int s = 3;
	for (int a = 1; a < (1 << s) * 4; a++) {
		printf("a=%2d ", a);
		for (int b = 0; b < a; b++) {
			printf("%2d ", calc(a, b, s));
			if (calc(a, b, s) != calc2(a, b, s)) puts("ERR");
		}
		printf("\n");
	}
}
