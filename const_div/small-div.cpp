// gcc, clang : -fopenmp
// VC : /openmp
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "constdiv.hpp"

uint32_t ceil_ilog2(uint32_t x)
{
	uint32_t a = 0;
	while ((one << a) < x) a++;
	return a;
}

// Division by Invariant Integers using Multiplication. Granlund, Montgomery
// https://dl.acm.org/doi/10.1145/178243.178249
struct GM {
	uint32_t d_;
	uint32_t l_;
	uint64_t mH_;
	uint32_t sh_;
	uint32_t a_; // same as ConstDiv
	void put() const
	{
		printf("GM d=%u(0x%08x) l=%u mH=0x%" PRIx64 " sh=0x%x a=%u\n", d_, d_, l_, mH_, sh_, a_);
	}
	bool init(uint32_t d)
	{
		uint32_t l = ceil_ilog2(d);
		uint32_t sh = l;
		uint64_t a = one << (N+l);
		uint64_t L = a / d;
//		uint64_t mH = (a + (one << l)) / d; // original Figure 6.2 of GM
		uint64_t mH = (a + (a / (M - (M % d)))) / d; // better parameter
		while ((L>>1) < (mH>>1) && sh > 0) {
			L >>= 1;
			mH >>= 1;
			sh--;
		}
		d_ = d;
		l_ = l;
		mH_ = mH;
		sh_ = sh;
		a_ = N + sh_;
		return true;
	}
	uint32_t divd(uint32_t x) const
	{
		if (mH_ > 0xffffffff) {
			uint32_t t1 = uint32_t(((mH_ - (M+1)) * x) >> N);
			uint32_t q = (t1 + ((x - t1) >> 1)) >> (sh_ - 1);
			return q;
		} else {
			uint32_t q = uint32_t((mH_ * x) >> (N + sh_));
			return q;
		}
	}
};

template<class Algo>
void check(const Algo& algo, uint32_t x)
{
	uint32_t q = algo.divd(x);
	uint32_t r = x - q * algo.d_;
	if (r >= algo.d_) {
		algo.put();
		printf("ERR x=%u d=%u ok=%u err=%u\n", x, algo.d_, x / algo.d_, q);
		exit(1);
	}
}

template<class Algo>
void checkAll(uint32_t d)
{
	Algo algo;
	if (!algo.init(d)) {
		printf("ERR algo.init(%u)\n", d);
		return;
	}
	algo.put();
	#pragma omp parallel for
	for (int64_t xx = 0; xx <= int64_t(M); xx++) {
		uint32_t x = uint32_t(xx);
		check(algo, x);
	}
}

template<class Algo>
void checkSomeP(const uint32_t *tbl, size_t tblN)
{
	for (size_t i = 0; i < tblN; i++) {
		uint32_t d = tbl[i];
		checkAll<Algo>(d);
	}
}

int main()
{
#if 0
	{
		ConstDivGen gen;
		gen.init(3);
		uint32_t q =gen.divd(15);
		printf("%d\n", q);
		gen.dump();
		checkAll<ConstDivGen>(3);
		return 0;
	}
#endif
	{
		const uint32_t tbl[] = { 3, 5, 7, 10, 13, 0x7ffff, 68641, 6864137, /* 0xffffffff, */ };
		const size_t tblN = sizeof(tbl) / sizeof(tbl[0]);
		checkSomeP<ConstDiv>(tbl, tblN);
//		checkSomeP<GM>(tbl, tblN);
		checkSomeP<ConstDivGen>(tbl, tblN);
	}
	// check ConstDiv
	{
		const uint32_t tbl[] = { 2, 4, 641, 65536, 0xb5062743, 0x7fffffff, 0x80000000, 0x80000001, 0xffffffff, };
		const size_t tblN = sizeof(tbl) / sizeof(tbl[0]);
		checkSomeP<ConstDiv>(tbl, tblN);
		checkSomeP<ConstDivGen>(tbl, tblN);
	}

#if 1
	{
		puts("find diff");
		#pragma omp parallel for
		for (int64_t d_ = 3; d_ <= 0xfffffffe; d_ += 2) {
			uint32_t d = uint32_t(d_);
			ConstDiv my;
			GM gm;
			bool b1 = my.init(d);
			bool b2 = gm.init(d);
			if (b1 && b2) {
				if (my.a_ != gm.a_) {
//					printf("d=%08x my=%u gm=%u\n", d, my.a_, gm.a_);
				}
				if (my.c_ <= M && gm.mH_ > M) {
					printf("d=0x%08x my.c=%" PRIx64 " a=%u gm.c=%" PRIx64 " a=%u\n", d, my.c_, my.a_, gm.mH_, gm.a_);
				}
			}
		}
		puts("ok");
	}
#endif


#if 1
	puts("all check");
	#pragma omp parallel for
	for (uint32_t d = 1; d <= 0xb0000000; d++) {
		ConstDiv algo;
		if (!algo.init(d)) {
			printf("ERR d=%u(0x%08x)\n", d, d);
//			checkAll(algo);
		}
	}
	puts("ok");
#endif
}
