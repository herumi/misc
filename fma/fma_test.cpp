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
	benchAll(n);
}


