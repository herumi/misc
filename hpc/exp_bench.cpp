/*
	This original source code is from http://www.chokkan.org/blog/archives/352

	g++ -O3 -march=native exp_bench.cpp

1000000 points by NormalRandomGenerator r(0, 1);

iCore i3-2120T@2.6GHz + Windows 7 + VC2013
exp                      55.38 1.635514436087156100e+005 0.000000e+000 0.000000e+000
fmath:exp(double)        31.29 1.635514436087156100e+005 6.239774e-016 1.120094e-016

iCore i3-2120T@2.6GHz + Linux + gcc-4.8.2
exp                     128.99 1.635514436087156064e+05 0.000000e+00 0.000000e+00
fmath:exp(double)        41.22 1.635514436087156064e+05 6.239774e-16 1.118914e-16

SPARC64 IXfx ; FCCpx -O3 -Kfast exp_bench.cpp fmath.cpp
FCCpx: Fujitsu C/C++ Compiler Driver Version 1.2.1 P-id: T01641-02 (Jul 29 2013 14:38:36)
exp                     128.82 1.635514436087156064e+05 0.000000e+00 0.000000e+00
fmath:exp(double)        36.94 1.635514436087156064e+05 7.572649e-16 1.774574e-16
*/
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include "cybozu/benchmark.hpp"
#include "fmath.hpp"

#ifdef _MSC_VER
#define MIE_ALIGN(x) __declspec(align(x))
#else
#define MIE_ALIGN(x) __attribute__((aligned(x)))
#endif

class RandomGenerator {
	unsigned int x_, y_, z_, w_;
public:
	RandomGenerator(int seed = 0)
	{
		init(seed);
	}
	void init(int seed = 0)
	{
		x_ = 123456789 + seed;
		y_ = 362436069;
		z_ = 521288629;
		w_ = 88675123;
	}
	unsigned int get()
	{
		unsigned int t = x_ ^ (x_ << 11);
		x_ = y_; y_ = z_; z_ = w_;
		return w_ = (w_ ^ (w_ >> 19)) ^ (t ^ (t >> 8));
	}
};
/*
	normal random generator
*/
class NormalRandomGenerator {
	RandomGenerator gen_;
	double u_;
	double s_;
public:
	NormalRandomGenerator(double u = 0, double s = 1, int seed = 0)
		: gen_(seed)
		, u_(u)
		, s_(s)
	{
	}
	void init(int seed = 0)
	{
		gen_.init(seed);
	}
	double get()
	{
		double sum = -6;
		for (int i = 0; i < 12; i++) {
			sum += gen_.get() / double(1ULL << 32);
		}
		return sum * s_ + u_;
	}
};

void bench(const char *msg, double (*f)(double), const std::vector<double>& v)
{
	const size_t n = v.size();
	cybozu::CpuClock clk;
	double sum = 0;
	clk.begin();
	for (size_t i = 0; i < n; i++) {
		sum += f(v[i]);
	}
	clk.end();
	double peak = 0;
	double rms = 0;
	for (size_t i = 0; i < n; i++) {
		double ex = f(v[i]);
		double exf = exp(v[i]);

		double err = fabs(exf - ex) / ex;
		if (peak < err) {
			peak = err;
		}
		rms += err * err;
	}

	rms /= n;
	rms = sqrt(rms);
	printf("%s %7.2f %.18e %e %e\n", msg, clk.getClock() / double(n), sum, peak, rms);
}

void compare(double x)
{
	double a = exp(x);
	double b = fmath::exp(x);
	double diff = fabs(a - b);
	if (a < 1e300 && diff > 1e-13) {
		double r = fabs(a - b) / a;
		if (r > 1e-13) printf("x=%.17g a=%.17g b=%.17g %.17g\n", x, a, b, r);
	}
}

void testLimits()
{
	const int N = 10000;
	for (int i = 0; i < N; i++) {
		double x = 709 + i / double(N);
		compare(x);
	}
	for (int i = 0; i < N; i++) {
		double x = -708 - i / double(N);
		compare(x);
	}
}

int main()
{
	fmath::init();
	testLimits();
	const struct {
		const char *name;
		double (*f)(double);
	} tbl[] = {
		{"exp                   ", exp },
		{"fmath:exp(double)     ", fmath::exp },
	};
	NormalRandomGenerator r(0, 1);
	std::vector<double> v(100000);

	for (size_t i = 0; i < v.size(); i++) {
		v[i] = r.get();
	}
	for (size_t i = 0; i < sizeof(tbl) / sizeof(*tbl); i++) {
		bench(tbl[i].name, tbl[i].f, v);
	}
}
