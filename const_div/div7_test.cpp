#include <stdio.h>
#include <stdint.h>
#include <cybozu/benchmark.hpp>

extern "C" {

uint32_t div7org(uint32_t x);
uint32_t div7a(uint32_t x);
uint32_t div7b(uint32_t x);

}

uint64_t sumAll(uint32_t (*f)(uint32_t), uint32_t n)
{
	uint64_t sum = 0;
	for (uint64_t x_ = 0; x_ <= n; x_++) {
		uint32_t x = uint32_t(x_);
		sum += f(x);
	}
	return sum;
}

int main()
{
	const uint32_t n = 100000000;
	const int C = 10;

	uint64_t sum0 = 0, sum1 = 0, sum2 = 0;
	CYBOZU_BENCH_C("org", C, sum0 += sumAll, div7org, n);
	CYBOZU_BENCH_C("a  ", C, sum1 += sumAll, div7a, n);
	CYBOZU_BENCH_C("b  ", C, sum2 += sumAll, div7b, n);
	CYBOZU_BENCH_C("org", C, sum0 += sumAll, div7org, n);
	printf("sum=%lx %lx %lx\n", sum0/2, sum1, sum2);
	#pragma omp parallel for
	for (uint64_t x_ = 0; x_ <= 0xffffffff; x_++) {
		uint32_t x = uint32_t(x_);
		uint32_t o = div7org(x);
		uint32_t a = div7a(x);
		uint32_t b = div7b(x);
		if (o != a || o != b) {
			printf("ERR x=%u o=%u a=%u b=%u\n", x, o, a, b);
		}
	}
	puts("ok");
}
