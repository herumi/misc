#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <cybozu/xorshift.hpp>
#include <cybozu/random_generator.hpp>
#include <cybozu/benchmark.hpp>
#if CYBOZU_CPP_VERSION == CYBOZU_CPP_VERSION_CPP11
#include <random>
#define USE_RANDOM
#endif
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>

#if defined(_MSC_VER) || defined(__RDRND__)
#define USE_RDRAND
uint32_t use_rdrand()
{
	for (int i = 0; i < 10; i++) {
		uint32_t x;
		if (_rdrand32_step(&x)) return x;
	}
	fprintf(stderr, "_rdrand32_step err\n");
	exit(1);
}
#endif

uint32_t use_xorshift()
{
	static cybozu::XorShift rg;
	return rg.get32();
}

uint32_t use_urandom()
{
	static cybozu::RandomGenerator rg;
	return rg.get32();
}

#ifdef USE_RANDOM
uint32_t use_mt()
{
	static std::mt19937 mt;
	return mt();
}
uint32_t use_random_device()
{
	static std::random_device rd;
	return rd();
}
#endif

int main()
{
#ifdef USE_RDRAND
	Xbyak::util::Cpu cpu;
	if (cpu.has(Xbyak::util::Cpu::tRDRAND)) {
		CYBOZU_BENCH("rdrand", use_rdrand);
	} else
#endif
	{
		puts("no rdrand");
	}
	CYBOZU_BENCH("xorshift", use_xorshift);
	CYBOZU_BENCH("urandom", use_urandom);
#ifdef USE_RANDOM
	CYBOZU_BENCH("mt19937", use_mt);
	CYBOZU_BENCH("random_device", use_random_device);
#endif
}

