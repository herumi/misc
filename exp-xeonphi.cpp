#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits>

#include <cybozu/benchmark.hpp>
#include <micvec.h>

#if defined(_WIN32) && !defined(__GNUC__)
	#include <intrin.h>
	#ifndef MIE_ALIGN
		#define MIE_ALIGN(x) __declspec(align(x))
	#endif
#else
	#ifndef __GNUC_PREREQ
	#define __GNUC_PREREQ(major, minor) ((((__GNUC__) << 16) + (__GNUC_MINOR__)) >= (((major) << 16) + (minor)))
	#endif
	#if __GNUC_PREREQ(4, 4) || !defined(__GNUC__)
		/* GCC >= 4.4 and non-GCC compilers */
		#include <x86intrin.h>
	#elif __GNUC_PREREQ(4, 1)
		/* GCC 4.1, 4.2, and 4.3 do not have x86intrin.h, directly include SSE2 header */
		#include <emmintrin.h>
	#endif
	#ifndef MIE_ALIGN
		#define MIE_ALIGN(x) __attribute__((aligned(x)))
	#endif
#endif
#ifndef MIE_PACK
	#define MIE_PACK(x, y, z, w) ((x) * 64 + (y) * 16 + (z) * 4 + (w))
#endif

#ifdef _MSC_VER
	#include <malloc.h>
#else
	#include <stdlib.h>
	static inline void *_aligned_malloc(size_t size, size_t alignment)
	{
		void *p;
		int ret = posix_memalign(&p, alignment, size);
		return (ret == 0) ? p : 0;
	}
#endif

void put(const void *p)
{
	const unsigned int *x = (const unsigned int*)p;
	for (int i = 0; i < 4; i++) {
		printf("%d:%08x %08x ", i, x[i * 2], x[i * 2 + 1]);
	}
	printf("\n\n");
}
#define PUT(x) printf("%s ", #x); put(&x);

namespace fmath {

namespace local {

const size_t EXP_TABLE_SIZE = 10;
const size_t EXPD_TABLE_SIZE = 11;
const size_t LOG_TABLE_SIZE = 12;

typedef unsigned long long uint64_t;

union fi {
	float f;
	unsigned int i;
};

union di {
	double d;
	uint64_t i;
};

inline unsigned int mask(int x)
{
	return (1U << x) - 1;
}

inline uint64_t mask64(int x)
{
	return (1ULL << x) - 1;
}

template<class T>
inline const T* cast_to(const void *p)
{
	return reinterpret_cast<const T*>(p);
}

template<class T, size_t N>
size_t NumOfArray(const T (&)[N]) { return N; }

template<size_t sbit_ = EXPD_TABLE_SIZE>
struct ExpdVar {
	enum {
		sbit = sbit_,
		s = 1UL << sbit,
		adj = (1UL << (sbit + 10)) - (1UL << sbit)
	};
	static const int EN = 8;
	// A = 1, B = 1, C = 1/2, D = 1/6
	double C1[EN]; // A
	double C2[EN]; // D
	double C3[EN]; // C/D
	uint64_t tbl[s];
	double a;
	double ra;
	ExpdVar()
		: a(s / ::log(2.0))
		, ra(1 / a)
	{
		for (int i = 0; i < EN; i++) {
#if 0
			C1[i] = 1.0;
			C2[i] = 0.16667794882310216;
			C3[i] = 2.9997969303278795;
#else
			C1[i] = 1.0;
			C2[i] = 0.16666666685227835064;
			C3[i] = 3.0000000027955394;
#endif
		}
		for (int i = 0; i < s; i++) {
			di di;
			di.d = ::pow(2.0, i * (1.0 / s));
			tbl[i] = di.i & mask64(52);
		}
	}
};

/* to define static variables in fmath.hpp */
template<size_t EXP_N = EXP_TABLE_SIZE, size_t LOG_N = LOG_TABLE_SIZE, size_t EXPD_N = EXPD_TABLE_SIZE>
struct C {
	static const ExpdVar<EXPD_N> expdVar;
};

template<size_t EXP_N, size_t LOG_N, size_t EXPD_N>
MIE_ALIGN(64) const ExpdVar<EXPD_N> C<EXP_N, LOG_N, EXPD_N>::expdVar;

__m512i shl32bit(__m512i x)
{
	static const MIE_ALIGN(64) int m[16] = { 0, 0, 0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14 };
	static const __m512i y = *(__m512i*)m;
	return _mm512_mask_permutevar_epi32(_mm512_setzero_epi32(), 0xaaaa, y, x);
}

} // local

inline double expd(double x)
{
	if (x <= -708.39641853226408) return 0;
	if (x >= 709.78271289338397) return std::numeric_limits<double>::infinity();
	using namespace local;
	const ExpdVar<>& c = C<>::expdVar;
	const uint64_t b = 3ULL << 51;
	di di;
	di.d = x * c.a + b;
	uint64_t iax = c.tbl[di.i & mask(c.sbit)];

	double t = (di.d - b) * c.ra - x;
	uint64_t u = ((di.i + c.adj) >> c.sbit) << 52;
	double y = (c.C3[0] - t) * (t * t) * c.C2[0] - t + c.C1[0];
//	double y = (2.999796930327879362111743 - t) * (t * t) * 0.166677948823102161853172 - t + 1.000000000000000000488181;

	di.i = u | iax;
	return y * di.d;
}

inline void expd_v(double *px, int n)
{
#if 0
	for (int i = 0; i < n / 8; i++) {
		*(__m512d*)&px[i * 8] = _mm512_exp_pd(*(const __m512d*)&px[i * 8]);
	}
#else
	using namespace local;
	const ExpdVar<>& c = C<>::expdVar;
	const double b = double(3ULL << 51);
	assert((n % 2) == 0);
	static const __m512d mC1 = *cast_to<__m512d>(c.C1);
	static const __m512d mC2 = *cast_to<__m512d>(c.C2);
	static const __m512d mC3 = *cast_to<__m512d>(c.C3);
	static const __m512d ma = _mm512_set1_pd(c.a);
	static const __m512d mra = _mm512_set1_pd(c.ra);
	static const __m512i madj = _mm512_set1_epi32(c.adj);
	static const __m512i mask_c_sbit = _mm512_set1_epi64(mask(c.sbit));
	static const __m512i mask20not = _mm512_set1_epi32(~mask(20));
	static const MIE_ALIGN(64) int m1[16] = { 0, 2, 4, 6, 8, 10, 12, 14 };
	static const __m512i selEven = *(__m512i*)m1;
	static const double cMax = 709.78271289338397;
	static const double cMin = -708.39641853226408 ;
	static const MIE_ALIGN(64) double expMax[] = { cMax, cMax, cMax, cMax, cMax, cMax, cMax, cMax };
	static const MIE_ALIGN(64) double expMin[] = { cMin, cMin, cMin, cMin, cMin, cMin, cMin, cMin };
	for (unsigned int i = 0; i < (unsigned int)n; i += 8) {
		__m512d x = _mm512_load_pd(px);
//_mm_prefetch((const char*)(px + 8), 0); // 25clk slow
//_mm_prefetch((const char*)(px + 8), 1); // 25clk slow
// 20.8clk -> 19
//		x = _mm512_min_pd(x, *(const __m512d*)expMax);
//		x = _mm512_max_pd(x, *(const __m512d*)expMin);

		__m512d d = _mm512_mul_pd(x, ma);
		d = _mm512_add_pd(d, _mm512_set1_pd(b));
		__m512i idx = _mm512_and_epi64(_mm512_castpd_si512(d), mask_c_sbit);
		idx = _mm512_permutevar_epi32(selEven, idx);
//PUT(idx);
		__m512i iax = _mm512_i32logather_epi64(idx, &c.tbl[0], 8);
//PUT(iax);
		__m512d t =_mm512_sub_pd(d, _mm512_set1_pd(b));
		t = _mm512_mul_pd(t, mra);
		t =_mm512_sub_pd(t, x);
		__m512i u = _mm512_castpd_si512(d);
		u = _mm512_add_epi64(u, madj);
		// (u >> 11) << 52 ==> ((u << 9) & ~mask(20)) << 32
		u = _mm512_and_epi64(_mm512_slli_epi32(u, 9), mask20not);
		u = local::shl32bit(u);
		u = _mm512_or_epi64(u, iax);
		__m512d y = _mm512_mul_pd(_mm512_sub_pd(mC3, t), _mm512_mul_pd(t, t));
		y = _mm512_mul_pd(y, mC2);
		y = _mm512_add_pd(_mm512_sub_pd(y, t), mC1);
		_mm512_store_pd(px, _mm512_mul_pd(y, _mm512_castsi512_pd(u)));
		px += 8;
	}
#endif
}

} // fmath::local

typedef struct {
	const char *name;
	void (*func)(double *values, int n);
	double error_peak;
	double error_rms;
	long long elapsed_time;
	double *values;
} performance_t;

void vecexp_libc(double *values, int n)
{
	int i;
	for (i = 0;i < n;++i) {
		values[i] = exp(values[i]);
	}
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

double *read_source(int *num)
{
	const int n = 1000000;
	*num = n;
	NormalRandomGenerator r(0, 1);
	double *values = (double*)malloc(n * sizeof(double));
	for (int i = 0; i < n; i++) {
		values[i] = r.get();
	}
	return values;
}

void measure(performance_t *perf, double *values, int n)
{
	int i;
	performance_t *p;

	for (p = perf;p->func != NULL;++p) {
		p->values = (double*)_aligned_malloc(sizeof(double) * n, 32);
		for (i = 0;i < n;++i) {
			p->values[i] = values[i];
		}
	}

	for (p = perf;p->func != NULL;++p) {
		cybozu::CpuClock clk;
		clk.begin();
		p->func(p->values, n);
		clk.end();
		p->elapsed_time = clk.getClock();
	}

	for (p = perf;p->func != NULL;++p) {
		for (i = 0;i < n;++i) {
			double ex = perf[0].values[i];
			double exf = p->values[i];

			double err = fabs(exf - ex) / ex;
			if (p->error_peak < err) {
				p->error_peak = err;
			}
			p->error_rms += (err * err);
		}

		p->error_rms /= n;
		p->error_rms = sqrt(p->error_rms);
	}
}

#if 1
void fmath_expd(double *values, int n)
{
	int i;
	for (i = 0;i < n;++i) {
		values[i] = fmath::expd(values[i]);
	}
}
#endif

void benchmark(const char *str, double f(double))
{
	double a = 0;
	cybozu::CpuClock clk;
	clk.begin();
	int n = 0;
	for (double x = 0; x < 1; x += 1e-8) {
		a += f(x);
		n++;
	}
	clk.end();
	printf("%s %.3fclk, a=%f\n", str, clk.getClock() / double(n), a);
}

void compare(double x)
{
	double a = exp(x);
	double b = 0;//fmath::expd(x);
	double diff = fabs(a - b);
	if (diff > 1e-13 && fabs(a - b) / a > 1e-13) {
		printf("x=%.17g a=%.17g b=%.17g\n", x, a, b);
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

void check()
{
	MIE_ALIGN(64) double x[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	fmath::expd_v(x, 8);
//	exit(1);
}

int main()
{
	check();
//	testLimits();
	benchmark("std::exp	", ::exp);
	benchmark("fmath::expd ", fmath::expd);

	int n;
	double *values = NULL;
	performance_t *p = NULL;

	performance_t perf[] = {
		{"libc				  ", vecexp_libc, 0., 0., 0, NULL},
		{"fmath_expd			", fmath_expd, 0., 0., 0, NULL},
		{"fmath_expd_v		  ", fmath::expd_v, 0., 0., 0, NULL},
		{NULL, NULL, 0., 0., 0, NULL},
	};

	values = read_source(&n);
	measure(perf, values, n);

	for (p = perf;p->func != NULL;++p) {
		printf(
			"%s\t%f\t%e\t%e\n",
			p->name,
			p->elapsed_time / (double)n,
			p->error_peak,
			p->error_rms
			);
	}
}

