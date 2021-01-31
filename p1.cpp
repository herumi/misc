/*
D=2 N=100
    c=338
D=3 N=1000
    c=6388
D=4 N=10000
    c=87320
D=5 N=100000
    c=973935
D=6 N=1000000
   c=12881436
D=7 N=10000000
   c=150248026
D=8 N=100000000
   c=1721663629
*/
#include <stdio.h>
#include <cybozu/xorshift.hpp>

constexpr size_t ipow(size_t n)
{
	size_t r = 1;
	for (size_t i = 0; i < n; i++) {
		r *= 10;
	}
	return r;
}

template<size_t D>
int count()
{
	constexpr size_t N = ipow(D);
	printf("D=%zd N=%zd\n", D, N);
	static int tbl[N] = {};
	cybozu::XorShift rg;
	size_t x = 0;
	size_t remain = N - 1;
	int c = 0;
	while (remain > 0) {
		int d = rg() % 10;
		x *= 10;
		x %= N;
		x += d;
		tbl[x]++;
		if (tbl[x] == 1) {
			remain--;
		}
		c++;
	}
	return c;
}

int main()
{
	printf("c=%d\n", count<2>());
	printf("c=%d\n", count<3>());
	printf("c=%d\n", count<4>());
	printf("c=%d\n", count<5>());
	printf("c=%d\n", count<6>());
	printf("c=%d\n", count<7>());
	printf("c=%d\n", count<8>());
}

