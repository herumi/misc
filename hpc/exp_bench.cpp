/*
	This original source code is from http://www.chokkan.org/blog/archives/352

	g++ -O3 -march=native exp_bench.cpp

1000000 points by NormalRandomGenerator r(0, 1);

iCore i3-2120T@2.6GHz + Windows 7 + VC2013
x=709.7826 a=1.7974901986562304e+308 b=1.#INF
x=709.78269999999998 a=1.7976699566638014e+308 b=1.#INF
exp                     49.07 1.635514436087156100e+005 0.000000e+000 0.000000e+000
Cephes                   61.44 1.635514436087156100e+005 2.922540e-016 5.740682e-017
Taylor 11th              69.36 1.635514436087154900e+005 8.635465e-015 1.369269e-015
Taylor 13th              80.53 1.635514436087156100e+005 2.203796e-016 4.950330e-017
Remez 11th [-0.5,+0.5]   69.36 1.635514436087156100e+005 4.710221e-016 1.945707e-016
Remez 13th [-0.5,+0.5]   79.77 1.635514436087156100e+005 2.203796e-016 4.955231e-017
Remez 11th [0,log2]      72.69 1.635514436087156100e+005 2.528194e-016 6.132200e-017
Remez 13th [0,log2]      86.62 1.635514436087156100e+005 2.357074e-016 6.097500e-017
fmath:expd               30.96 1.635514436087156100e+005 6.239774e-016 1.120094e-016
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

typedef union {
	double d;
	unsigned short s[4];
} ieee754;

static const double LOG2E  =  1.4426950408889634073599;	 /* 1/log(2) */
static const double C1 = 6.93145751953125E-1;
static const double C2 = 1.42860682030941723212E-6;

double remez11_0_log2(double x)
{
	int n;
	double a, px;
	ieee754 u;

	/* n = floor(x / log 2) */
	a = LOG2E * x;
	a -= (a < 0);
	n = (int)a;

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;

	/* Compute e^x using a polynomial approximation. */
	a = 3.552462518547823266595e-8;
	a *= x;
	a += 2.55353685193065003433e-7;
	a *= x;
	a += 2.77750562801295315877e-6;
	a *= x;
	a += 2.47868893393199945541e-5;
	a *= x;
	a += 1.98419213985637881240e-4;
	a *= x;
	a += 1.38888696841786592390e-3;
	a *= x;
	a += 8.33333370520098722211e-3;
	a *= x;
	a += 4.16666666210808106103e-2;
	a *= x;
	a += 0.16666666666996080348;
	a *= x;
	a += 0.49999999999987709448;
	a *= x;
	a += 1.00000000000000179527;
	a *= x;
	a += 0.99999999999999999566;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return a * u.d;
}

double remez13_0_log2(double x)
{
	int n;
	double a, px;
	ieee754 u;

	/* n = floor(x / log 2) */
	a = LOG2E * x;
	a -= (a < 0);
	n = (int)a;

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;

	/* Compute e^x using a polynomial approximation. */
	a = 2.276293252915046061949e-10;
	a *= x;
	a += 1.93367224471636363463e-9;
	a *= x;
	a += 2.52543927629810215309e-8;
	a *= x;
	a += 2.75401448018606365164e-7;
	a *= x;
	a += 2.75583147053220552447e-6;
	a *= x;
	a += 2.48015469521962683865e-5;
	a *= x;
	a += 1.98412709907914555147e-4;
	a *= x;
	a += 1.38888888661019235625e-3;
	a *= x;
	a += 8.33333333363959874817e-3;
	a *= x;
	a += 4.16666666666400203177e-2;
	a *= x;
	a += 0.16666666666666805461;
	a *= x;
	a += 0.49999999999999996228;
	a *= x;
	a += 1.00000000000000000040;
	a *= x;
	a += 0.99999999999999999999;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return a * u.d;
}

double exp_remez11_05_05(double x)
{
	int n;
	double a, px;
	ieee754 u;

	/* n = round(x / log 2) */
	a = LOG2E * x + 0.5;
	n = (int)a;
	n -= (a < 0);

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;

	/* Compute e^x using a polynomial approximation. */
	a = 2.519298689585589901007e-8;
	a *= x;
	a += 2.77143029724764708572e-7;
	a *= x;
	a += 2.75568408339931215724e-6;
	a *= x;
	a += 2.48011454027599465130e-5;
	a *= x;
	a += 1.98412706284313544001e-4;
	a *= x;
	a += 1.38888894619673011407e-3;
	a *= x;
	a += 8.33333333269606253027e-3;
	a *= x;
	a += 4.16666666633079258298e-2;
	a *= x;
	a += 0.16666666666668912253;
	a *= x;
	a += 0.50000000000007198509;
	a *= x;
	a += 0.99999999999999976925;
	a *= x;
	a += 0.99999999999999975002;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return a * u.d;
}

double exp_remez13_05_05(double x)
{
	int n;
	double a, px;
	ieee754 u;

	/* n = round(x / log 2) */
	a = LOG2E * x + 0.5;
	n = (int)a;
	n -= (a < 0);

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;

	/* Compute e^x using a polynomial approximation. */
	a = 1.613568489917573961956e-10;
	a *= x;
	a += 2.09773502429720720042e-9;
	a *= x;
	a += 2.50517997973487340520e-8;
	a *= x;
	a += 2.75569731779054668778e-7;
	a *= x;
	a += 2.75573198608708681627e-6;
	a *= x;
	a += 2.48015878916569323181e-5;
	a *= x;
	a += 1.98412698405602139306e-4;
	a *= x;
	a += 1.38888888883724638191e-3;
	a *= x;
	a += 8.33333333333374324309e-3;
	a *= x;
	a += 4.16666666666688187524e-2;
	a *= x;
	a += 0.16666666666666665609;
	a *= x;
	a += 0.49999999999999996637;
	a *= x;
	a += 1.00000000000000000008;
	a *= x;
	a += 1.00000000000000000008;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return a * u.d;
}

double exp_taylor11(double x)
{
	int n;
	double a, px;
	ieee754 u;

	/* n = round(x / log 2) */
	a = LOG2E * x + 0.5;
	n = (int)a;
	n -= (a < 0);

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;

	/* Compute e^x using a polynomial approximation. */
	a = 1. / 39916800.;
	a *= x;
	a += 2.75573192239858906525e-7;
	a *= x;
	a += 2.75573192239858906525e-6;
	a *= x;
	a += 2.48015873015873015873e-5;
	a *= x;
	a += 1.98412698412698412698e-4;
	a *= x;
	a += 1.38888888888888888888e-3;
	a *= x;
	a += 8.33333333333333333333e-3;
	a *= x;
	a += 4.16666666666666666666e-2;
	a *= x;
	a += 0.16666666666666666666;
	a *= x;
	a += 0.5;
	a *= x;
	a += 1.0;
	a *= x;
	a += 1.0;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return a * u.d;
}

double exp_taylor13(double x)
{
	int n;
	double a, px;
	ieee754 u;

	/* n = round(x / log 2) */
	a = LOG2E * x + 0.5;
	n = (int)a;
	n -= (a < 0);

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;

	/* Compute e^x using a polynomial approximation. */
	a = 1. / 6227020800LL;
	a *= x;
	a += 2.08767569878680989792e-9;
	a *= x;
	a += 2.50521083854417187750e-8;
	a *= x;
	a += 2.75573192239858906525e-7;
	a *= x;
	a += 2.75573192239858906525e-6;
	a *= x;
	a += 2.48015873015873015873e-5;
	a *= x;
	a += 1.98412698412698412698e-4;
	a *= x;
	a += 1.38888888888888888888e-3;
	a *= x;
	a += 8.33333333333333333333e-3;
	a *= x;
	a += 4.16666666666666666666e-2;
	a *= x;
	a += 0.16666666666666666666;
	a *= x;
	a += 0.5;
	a *= x;
	a += 1.0;
	a *= x;
	a += 1.0;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return a * u.d;
}

double exp_cephes(double x)
{
	int n;
	double a, xx, px, qx;
	ieee754 u;

	/* n = round(x / log 2) */
	a = LOG2E * x + 0.5;
	n = (int)a;
	n -= (a < 0);

	/* x -= n * log2 */
	px = (double)n;
	x -= px * C1;
	x -= px * C2;
	xx = x * x;

	/* px = x * P(x**2). */
	px = 1.26177193074810590878E-4;
	px *= xx;
	px += 3.02994407707441961300E-2;
	px *= xx;
	px += 9.99999999999999999910E-1;
	px *= x;

	/* Evaluate Q(x**2). */
	qx = 3.00198505138664455042E-6;
	qx *= xx;
	qx += 2.52448340349684104192E-3;
	qx *= xx;
	qx += 2.27265548208155028766E-1;
	qx *= xx;
	qx += 2.00000000000000000009E0;

	/* e**x = 1 + 2x P(x**2)/( Q(x**2) - P(x**2) ) */
	x = px / (qx - px);
	x = 1.0 + 2.0 * x;

	/* Build 2^n in double. */
	u.d = 0;
	n += 1023;
	u.s[3] = (unsigned short)((n << 4) & 0x7FF0);

	return x * u.d;
}

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
	double b = fmath::expd(x);
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
	testLimits();
	const struct {
		const char *name;
		double (*f)(double);
	} tbl[] = {
		{"exp                   ", exp },
		{"Cephes                ", exp_cephes },
		{"Taylor 11th           ", exp_taylor11 },
		{"Taylor 13th           ", exp_taylor13 },
		{"Remez 11th [-0.5,+0.5]", exp_remez11_05_05 },
		{"Remez 13th [-0.5,+0.5]", exp_remez13_05_05 },
		{"Remez 11th [0,log2]   ", remez11_0_log2 },
		{"Remez 13th [0,log2]   ", remez13_0_log2 },
		{"fmath:expd            ", fmath::expd },
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
