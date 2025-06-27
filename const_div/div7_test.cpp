#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#define CYBOZU_BENCH_CHRONO
#include <cybozu/benchmark.hpp>
#include <cybozu/option.hpp>
#include "constdiv.hpp"

extern "C" {

uint32_t div7org(uint32_t x);
uint32_t div7org2(uint32_t x);
uint32_t div7a(uint32_t x);

uint32_t div7b(uint32_t x);

} // extern "C"

uint32_t loop1(DivFunc f, uint32_t n)
{
	uint32_t sum = 0;
	for (uint32_t x = 0; x < n; x++) {
		sum += f(x);
	}
	return sum;
}

uint32_t loop2(DivFunc f, uint32_t n)
{
	uint32_t x = 123;
	for (uint32_t i = 0; i < n; i++) {
		x = x * f(x) + 12345;
	}
	return x;
}

DivFunc gen = 0;

void loopTest(const char *msg, uint32_t (*loop)(DivFunc f, uint32_t n), const DivFunc *divLp)
{
	const uint32_t n = 20000000;
	const int C = 100;

	uint32_t r0 = 0, r1 = 0;;
	puts(msg);
	CYBOZU_BENCH_C("org ", C, r0 += loop, div7org, n);
	CYBOZU_BENCH_C("org2", C, r1 += loop, div7org2, n);
	uint32_t rs[FUNC_N] = {};
	if (gen) {
		for (size_t i = 0; i < FUNC_N; i++) {
			DivFunc f = divLp[i];
			char buf[100];
			snprintf(buf, sizeof(buf), "lp%zu ", i);
			CYBOZU_BENCH_C(buf, C, rs[i] += f, n);
		}
	}
	printf("org  =0x%08x\n", r0);
	printf("org2 =0x%08x\n", r1);
	for (size_t i = 0; i < FUNC_N; i++) {
		printf("rs[%zd]=0x%08x\n", i, rs[i]);
	}
}

int main(int argc, char *argv[])
{
	cybozu::Option opt;
//	opt.appendOpt(&g_mode, 0, "m", "mode");
	opt.appendHelp("h");
	if (opt.parse(argc, argv)) {
		opt.put();
	} else {
		opt.usage();
	}

#ifdef CONST_DIV_GEN
	ConstDivGen cdg;
	cdg.init(7);
	cdg.put();
	cdg.dump();
	gen = cdg.divd;
#endif
	loopTest("loop1", loop1, cdg.divLp);
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
