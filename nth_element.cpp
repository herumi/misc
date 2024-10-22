#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cybozu/xorshift.hpp>
#include <cybozu/benchmark.hpp>

typedef std::vector<int> IntVec;

void init(IntVec& v, cybozu::XorShift& rg)
{
	for (size_t i = 0; i < v.size(); i++) {
		v[i] = rg.get32();
	}
}

void term(const IntVec& v)
{
	volatile int sum = std::accumulate(v.begin(), v.end(), 0);
	(void)sum;
}

void dump(const IntVec& v)
{
	for (size_t i = 0; i < v.size(); i++) printf("%d ", v[i]);
	printf("\n");
}

void test0(cybozu::XorShift& rg, IntVec& v)
{
	init(v, rg);
	term(v);
}

int cmp(const void *p, const void *q)
{
	int a = *(const int*)p;
	int b = *(const int*)q;
	if (a > b) return 1;
	if (a == b) return 0;
	return -1;
}

void test1(int& ret, cybozu::XorShift& rg, IntVec& v)
{
	init(v, rg);
	const size_t n = v.size();
	qsort(v.data(), n, sizeof(int), cmp);
	ret = v[n/2];
	term(v);
}

void test2(int& ret, cybozu::XorShift& rg, IntVec& v)
{
	init(v, rg);
	const size_t n = v.size();
	std::sort(v.begin(), v.end());
	ret = v[n/2];
	term(v);
}

void test3(int& ret, cybozu::XorShift& rg, IntVec& v)
{
	init(v, rg);
	const size_t n = v.size();
	std::nth_element(v.begin(), v.begin() + n/2, v.end());
	ret = v[n/2];
	term(v);
}

int main()
{
	size_t n = 1000;
	for (size_t i = 0; i < 4; i++) {
		printf("n=%zd\n", n);
		const int C = 100;
		IntVec v(n);
		int ret;
		{ cybozu::XorShift rg; CYBOZU_BENCH_C("base ", C, test0, rg, v); }
		{ cybozu::XorShift rg; CYBOZU_BENCH_C("qsort", C, test1, ret, rg, v); }
		{ cybozu::XorShift rg; CYBOZU_BENCH_C("sort ", C, test2, ret, rg, v); }
		{ cybozu::XorShift rg; CYBOZU_BENCH_C("nth_e", C, test3, ret, rg, v); }
		n *= 10;
	}
}
