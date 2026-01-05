/*
Linx:
g++ openmp-reduction2.cpp -O3 -fopenmp

Mac
clang++ openmp-reduction2.cpp -O3 -Xpreprocessor -fopenmp -lomp -L /opt/homebrew/opt/libomp/lib/

Windows: MSVC does not support custom reduction (OpenMP 4.0 or later).
clang-cl openmp-reduction2.cpp -O3 -openmp
cl /Ox /openmp:llvm openmp-reduction2.cpp
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <atomic>
#include <chrono>

#ifndef _MSC_VER
	#define USE_OPENMP_CUSTOM_REDUCTION
#endif

typedef std::chrono::high_resolution_clock::time_point TimePoint;
TimePoint getNow()
{
	return std::chrono::high_resolution_clock::now();
}

void putTime(const TimePoint& start)
{
	TimePoint now = getNow();
	uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	printf("time=%.3f msec\n", double(duration)*1e-9 * 1e3);
}

const int MAX = 0x7fffffff;

int get(int x)
{
	double v = x / double(MAX);
	for (int i = 0; i < 5; i++) {
		v += sin(v);
	}
	return (int)(v*1234567) + 1;
}

void check1()
{
	for (int i = 0; i < MAX; i++) {
		if (get(i) == 0) {
			printf("i=%d\n", i); exit(1);
		}
	}
	puts("ok");
}

void check2()
{
	#pragma omp parallel for
	for (int i = 0; i < MAX; i++) {
		if (get(i) == 0) {
			printf("i=%d\n", i); exit(1);
		}
	}
	puts("ok");
}

/*
m=3878506
time=77583.424 msec
77.582u 0.004s 1:17.58 100.0%   0+0k 0+0io 0pf+0w
*/
void test0()
{
	int m = 0;
	for (int i = 0; i < MAX; i++) {
		int v = get(i);
		if (v > m) {
			m = v;
		}
	}
	printf("m=%d\n", m);
}

/*
m=3878506
time=1756.609 msec
181.652u 0.455s 0:01.76 10346.5%        0+0k 0+0io 110pf+0w
*/
void test1()
{
	int m = 0;
	#pragma omp parallel for reduction(max:m)
	for (int i = 0; i < MAX; i++) {
		int v = get(i);
		if (v > m) {
			m = v;
		}
	}
	printf("m=%d\n", m);
}

/*
m=3878506 mi=1000023228
time=77583.044 msec
77.585u 0.000s 1:17.58 100.0%   0+0k 0+0io 0pf+0w
*/
void test2()
{
	int m = 0;
	int mi = 0;
	for (int i = 0; i < MAX; i++) {
		int v = get(i);
		if (v > m) {
			m = v;
			mi = i;
		}
	}
	printf("m=%d mi=%d v=%d\n", m, mi, get(mi));
}

/*
m=3878506 mi=2013265920 v=3878506
time=1749.365 msec
181.267u 0.003s 0:01.75 10357.7%        0+0k 0+0io 125pf+0w
*/
void test3()
{
	int m = 0;
	int mi = 0;
	#pragma omp parallel
	{
		int local_m = 0;
		int local_mi = 0;
		#pragma omp for
		for (int i = 0; i < MAX; i++) {
			int v = get(i);
			if (v > local_m) {
				local_m = v;
				local_mi = i;
			}
		}
		#pragma omp critical
		{
			if (local_m > m) {
				m = local_m;
				mi = local_mi;
			}
		}
	}
	printf("m=%d mi=%d v=%d\n", m, mi, get(mi));
}

/*
m=3878506 mi=1000023228 v=3878506
time=1785.982 msec
184.989u 0.035s 0:01.78 10393.8%        0+0k 0+0io 108pf+0w
*/
void test4()
{
	int m = 0;
	int mi = 0;
	#pragma omp parallel
	{
		int local_m = 0;
		int local_mi = 0;
		#pragma omp for
		for (int i = 0; i < MAX; i++) {
			int v = get(i);
			if (v > local_m) {
				local_m = v;
				local_mi = i;
			}
			if (v == local_m && i < local_mi) {
				local_mi = i;
			}
		}
		#pragma omp critical
		{
			if (local_m > m) {
				m = local_m;
				mi = local_mi;
			}
			if (m == local_m && local_mi < mi) {
				mi = local_mi;
			}
		}
	}
	printf("m=%d mi=%d v=%d\n", m, mi, get(mi));
}

struct Stat {
	int m = 0;
	int mi = 0;
	void combine(const Stat& rhs)
	{
		if (rhs.m > m) {
			m = rhs.m;
			mi = rhs.mi;
		}
		if (rhs.m == m && rhs.mi < mi) {
			mi = rhs.mi;
		}
	}
};

/*
m=3878506 mi=1000023228 v=3878506
time=1770.307 msec
183.629u 0.015s 0:01.77 10374.5%        0+0k 0+0io 110pf+0w
*/
void test5()
{
	Stat stat;
	#pragma omp parallel
	{
		Stat local;
		#pragma omp for
		for (int i = 0; i < MAX; i++) {
			int v = get(i);
			local.combine(Stat{v, i});
		}
		#pragma omp critical
		stat.combine(local);
	}
	int m = stat.m;
	int mi = stat.mi;
	printf("m=%d mi=%d v=%d\n", m, mi, get(mi));
}

#ifdef USE_OPENMP_CUSTOM_REDUCTION
/*
m=3878506 mi=1000023228 v=3878506
time=1767.110 msec
183.224u 0.008s 0:01.77 10351.4%        0+0k 0+0io 107pf+0w
*/
void test6()
{
	Stat stat;
	#pragma omp declare reduction(stat_red: Stat: omp_out.combine(omp_in)) initializer(omp_priv = Stat())
	#pragma omp parallel for reduction(stat_red:stat)
	for (int i = 0; i < MAX; i++) {
		int v = get(i);
		stat.combine(Stat{v, i});
	}
	int m = stat.m;
	int mi = stat.mi;
	printf("m=%d mi=%d v=%d\n", m, mi, get(mi));
}
#endif

int main(int argc, char *argv[])
{
	const int mode = argc == 1 ? 0 : atoi(argv[1]);
	printf("mode=%d\n", mode);
	TimePoint start = getNow();
	switch (mode) {
	case -2: check2(); break;
	case -1: check1(); break;
	case 0: test0(); break;
	case 1: test1(); break;
	case 2: test2(); break;
	case 3: test3(); break;
	case 4: test4(); break;
	case 5: test5(); break;
#ifdef USE_OPENMP_CUSTOM_REDUCTION
	case 6: test6(); break;
#endif
	default: puts("not supported"); return 1;
	}
	putTime(start);
}
