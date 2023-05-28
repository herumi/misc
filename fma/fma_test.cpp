#include <cybozu/benchmark.hpp>
#include <cybozu/option.hpp>

#include "func.h"

const int N = 5*7*8*9*1000;

template<int n>
void bench()
{
	int C = 100;
	CYBOZU_BENCH_C("", C, funcN<n>, N);
    printf("n=%d %f\n", n, cybozu::bench::g_clk.getClock()/double(N)/C);
}

int main(int argc, char *argv[])
{
	cybozu::Option opt;
	int n;
	opt.appendOpt(&n, 0, "n", "unroll n");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 0;
	}
	if (n > 0) {
		switch (n) {
		case 4: bench<4>(); break;
		case 7: bench<7>(); break;
		case 8: bench<8>(); break;
		default:
			printf("not support n=%d\n", n);
			break;
		}
		return 0;
	}
	bench<1>();
	bench<2>();
	bench<3>();
	bench<4>();
	bench<5>();
	bench<6>();
	bench<7>();
	bench<8>();
	bench<9>();
	bench<10>();
	bench<11>();
	bench<12>();
	bench<13>();
	bench<14>();
	bench<15>();
	bench<16>();
}


