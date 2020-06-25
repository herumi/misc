#include <stdio.h>
#include <math.h>
#include <stdint.h>

union fi {
	float f;
	uint32_t i;
};

uint32_t mask(int n)
{
	if (n == 32) return uint32_t(-1);
	return (1u << n) - 1;
}

struct FexpaTbl {
	static const size_t N = 64;
	uint32_t tbl[N];
	FexpaTbl()
	{
		for (int i = 0; i < N; i++) {
			fi fi;
			fi.f = (float)pow(2.0, i / float(N));
			tbl[i] = fi.i & mask(23);
		}
	}
};

/*
e^x = 2^(log_2(e) x)
y = log_2(e) x
z = y - 1
n = floor(z)
a = z - n ; 0 <= a < 1
b = 1 + a ; 1 <= b < 2, y = z + 1 = (n + a) + 1 = n + b
b1 = (b & (mask(32) << 17)
b2 = (b & ~(mask(6) << 17)) - 1 ; b = b1 + b2
d = fexpa[b1] ; d = 2^b1
f = b2 * loge_2
g = 1 + f + f^2/2 + f^3/6

e^x = 2^y = 2^(n + b1 + b2)
= (2^n) (2^b1) (2^b2)
= (2^n) (2^b1) e^f
*/
float expC(float x)
{
	static const float log2 = log(2.0f);
	static const float log2_e = 1 / log2;
}

int main()
{
	FexpaTbl tbl;
	for (int i = 0; i < 64; i++) {
		printf("%d %08x\n", i, tbl.tbl[i]);
	}
}
