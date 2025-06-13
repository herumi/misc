#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#ifndef _MSC_VER
#define CYBOZU_BENCH_USE_GETTIMEOFDAY
#endif
#include <cybozu/benchmark.hpp>
#include <cybozu/option.hpp>
#include "constdiv.hpp"

int g_mode;

extern "C" {

uint32_t div7org(uint32_t x);
uint32_t div7a(uint32_t x);

uint32_t div7b(uint32_t x);

} // extern "C"

uint64_t loop1(uint32_t (*f)(uint32_t), uint32_t n)
{
	uint64_t sum = 0;
	for (uint32_t x = 0; x < n; x++) {
		sum += f(x);
	}
	return sum;
}

uint64_t loop2(uint32_t (*f)(uint32_t), uint32_t n)
{
	uint32_t x = 123;
	for (uint32_t i = 0; i < n; i++) {
		x = x * f(x) + 12345;
	}
	return x;
}

int main(int argc, char *argv[])
{
	const uint32_t n = 10000000;
	const int C = 100;

	uint64_t r0 = 0, r1 = 0, r2 = 0, r3 = 0;

	cybozu::Option opt;
	opt.appendOpt(&g_mode, 0, "m", "mode");
	opt.appendHelp("h");
	if (opt.parse(argc, argv)) {
		opt.put();
	} else {
		opt.usage();
	}

	ConstDivGen cdg;
	cdg.init(7);
	cdg.dump();
	puts("loop1");
	ConstDivGen::DivFunc f = cdg.divd;
	CYBOZU_BENCH_C("org", C, r0 += loop1, div7org, n);
	CYBOZU_BENCH_C("a  ", C, r1 += loop1, div7a, n);
	CYBOZU_BENCH_C("b  ", C, r2 += loop1, div7b, n);
	CYBOZU_BENCH_C("cdg", C, r3 += loop1, f, n);
	CYBOZU_BENCH_C("org", C, r0 += loop1, div7org, n);
	CYBOZU_BENCH_C("a  ", C, r1 += loop1, div7a, n);
	CYBOZU_BENCH_C("b  ", C, r2 += loop1, div7b, n);
	CYBOZU_BENCH_C("cdg", C, r3 += loop1, f, n);
	printf("sum=%" PRIx64 " %" PRIx64 " %" PRIx64 " %" PRIx64 "\n", r0, r1, r2, r3);
	if ((r0 ^ r1) | (r0 ^ r2) | (r0 ^ r3)) printf("ERR\n");
	puts("loop2");
	CYBOZU_BENCH_C("org", C, r0 += loop2, div7org, n);
	CYBOZU_BENCH_C("a  ", C, r1 += loop2, div7a, n);
	CYBOZU_BENCH_C("b  ", C, r2 += loop2, div7b, n);
	CYBOZU_BENCH_C("cdg", C, r3 += loop2, f, n);
	CYBOZU_BENCH_C("org", C, r0 += loop2, div7org, n);
	CYBOZU_BENCH_C("a  ", C, r1 += loop2, div7a, n);
	CYBOZU_BENCH_C("b  ", C, r2 += loop2, div7b, n);
	CYBOZU_BENCH_C("cdg", C, r3 += loop2, f, n);
	printf("sum=%" PRIx64 " %" PRIx64 " %" PRIx64 " %" PRIx64 "\n", r0, r1, r2, r3);
	if ((r0 ^ r1) | (r0 ^ r2) | (r0 ^ r3)) printf("ERR\n");
#if 0
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
#endif
}
