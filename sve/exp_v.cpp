#include "fmath-sve.hpp"
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <vector>
#include "fexpa.hpp"
#include <float.h>

float g_maxe;

float diff(float x, float y)
{
	return std::abs(x - y) / x;
}

float fmath_expf(float x)
{
	float y[16] = { x };
	fmath::expf_v(y, y, 1);
	return y[0];
}

float fmath_tanhf(float x)
{
	float y[16] = { x };
	fmath::tanhf_v(y, y, 1);
	return y[0];
}

float u2f(uint32_t x)
{
	fmath::local::fi fi;
	fi.i = x;
	return fi.f;
}

uint32_t f2u(float x)
{
	fmath::local::fi fi;
	fi.f = x;
	return fi.i;
}

inline float split(int *pn, float x)
{
	int n;
	if (x >= 0) {
		n = int(x + 0.5f);
	} else {
		n = int(x - 0.5f);
	}
	*pn = n;
	return x - n;
}

inline float expfC(float x)
{
	using namespace fmath;
	const local::ConstVar& C = *local::Inst<>::code.constVar;

	float y = x * C.log2_e;
	int n = (int)floor(y);
	float a = y - n; // 0 <= a < 1
	float b = 1 + a; // 1 <= b < 2, y = (n-1) + b
	uint32_t bu = f2u(b);

	float bL = u2f(bu >> 17);
	float z = b - u2f(bu & C.not_mask17);
	/*
		split b into bL and z where bL is for fexpa and z is remain
	*/
	float c = fexpaEmu(bL);
	float d = 1 + (C.coeff1 + z * C.coeff2) * z;
	return powf(2.0f, n) * c * d;
}

inline float tanhfC(float x)
{
	float y = expfC(x * 2);
	return 1 - 2 / (1 + y);
}

void std_exp_v(float *dst, const float *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		dst[i] = std::exp(src[i]);
	}
}

void std_tanh_v(float *dst, const float *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		dst[i] = std::tanh(src[i]);
	}
}

#if 1
CYBOZU_TEST_AUTO(preciseTanh)
{
	float tbl[] = { -INFINITY, -100, -1, -0.6, -0.5, -0.4, -0.1, -0.000151307, 0.000151307, 0.1, 0.4, 0.5, 0.6, 1, 100, INFINITY };
	const size_t n = CYBOZU_NUM_OF_ARRAY(tbl);
	for (size_t i = 0; i < n; i++) {
		float x = tbl[i];
		float a = tanh(x);
		float b = fmath_tanhf(x);
		float e = x ? fabs(a - b) / x : fabs(a - b);
//		printf("%zd x=%e a=%e b=%e e=%e\n", i, x, a, b, e);
		CYBOZU_TEST_ASSERT(e < 1e-5);
	}
	for (float x = 1e-7; x < 0.1; x += 1e-4) {
		float a = tanh(x);
		float b = fmath_tanhf(x);
		float e = x ? fabs(a - b) / x : fabs(a - b);
		CYBOZU_TEST_ASSERT(e < 1e-5);
	}
}
#endif

#if 1
template<class F>
float putDiff(float begin, float end, float step, const F& f, bool doPut = false, float stdf(float) = std::exp)
{
	float maxe = 0;
	float maxx = 0;
	float maxe2 = 0;
	float maxx2 = 0;
	double ave = 0;
	int aveN = 0;
	for (float x = begin; x < end; x += step) {
		float y0 = stdf(x);
//		float y0 = expfC(x);
		float y1 = f(x);
		float e;
		e = diff(y0, y1);
		if (doPut) {
			printf("x=%.2e y0=%.2e(%08x) y1=%.2e(%08x)\n", x, y0, f2u(y0), y1, f2u(y1));
		}
		if (e > maxe) {
			maxe = e;
			maxx = x;
		}
		float e2 = fabs(y0 - y1);
		if (e2 > maxe2) {
			maxe2 = e2;
			maxx2 = x;
		}
		ave += e;
		aveN++;
	}
	printf("range [%.2e, %.2e] step=%.2e\n", begin, end, step);
	printf("maxe =%e (x=%e)\n", maxe, maxx);
	printf("maxe2=%e (x=%e)\n", maxe2, maxx2);
	printf("ave=%e\n", ave / aveN);
	return maxe;
}

CYBOZU_TEST_AUTO(tanh)
{
	puts("tanh");
	puts("fmath::tanhf_v");
	putDiff(-4, 4, 1e-5, fmath_tanhf, false, std::tanh);
}

CYBOZU_TEST_AUTO(setMaxE)
{
	puts("expfC");
	putDiff(-10, 10, 0.5, expfC);
	putDiff(-30, 30, 1e-5, expfC);
	puts("fmath::expf_v");
	putDiff(-10, 10, 0.5, fmath_expf);
	g_maxe = putDiff(-30, 30, 1e-5, fmath_expf);
}

void checkDiff(const float *x, const float *y, size_t n, bool put = false)
{
	for (size_t i = 0; i < n; i++) {
		float d = diff(x[i], y[i]);
		if (put) {
			if (d > g_maxe) {
				printf("err n=%zd, i=%zd x=%e y=%e\n", n, i, x[i], y[i]);
				exit(1);
			}
		} else {
			CYBOZU_TEST_ASSERT(d <= g_maxe);
		}
	}
}

CYBOZU_TEST_AUTO(expf_v)
{
	const size_t n = 300;
	float x[n];
	float y1[n];
	float y2[n];
	for (size_t i = 0; i < n; i++) {
		x[i] = float((i - n/2.0) / n * 20);
	}
	std_exp_v(y1, x, n);
	fmath::expf_v(y2, x, n);
	checkDiff(y1, y2, n);
}

typedef std::vector<float> Fvec;

void putClk(const char *msg, size_t n)
{
	printf("%s %.2fnsec\n", msg, cybozu::bench::g_clk.getClock() / double(n));
}

CYBOZU_TEST_AUTO(bench)
{
	Fvec x, y0, y1;
	size_t n = 1024 * 16;
	x.resize(n);
	y0.resize(n);
	y1.resize(n);
	const int C = 30000;
	for (size_t i = 0; i < n; i++) {
		x[i] = sin(i / double(n) * 7) * 20;
	}
	printf("for float x[%zd];\n", n);
	CYBOZU_BENCH_C("", C, std_exp_v, &y0[0], &x[0], n);
	putClk("std::exp", C * (n / 16));
	for (int i = 0; i < 100; i++) {
		memset(&y1[0], 0, i * sizeof(float));
		fmath::expf_v(&y1[0], &x[0], i);
		checkDiff(&y0[0], &y1[0], i);
	}
	CYBOZU_BENCH_C("", C, fmath::expf_v, &y1[0], &x[0], n);
	putClk("fmath::expf_v", C * (n / 16));
	checkDiff(&y0[0], &y1[0], n);
	n = 1024 * 4;
	memset(y1.data(), 0, n * sizeof(float));
	CYBOZU_BENCH_C("", C, fmath::expf_v, &y1[0], &x[0], n);
	putClk("fmath::expf_v", C * (n / 16));
	checkDiff(&y0[0], &y1[0], n);
	CYBOZU_BENCH_C("", C, fmath::tanhf_v, &y1[0], &x[0], n);
	putClk("fmath::tanhf_v", C * (n / 16));
}

void limitTest(float f1(float), float f2(float))
{
	float tbl[] = { 0, FLT_MIN, 0.5, 1,  80, 100, 1000, FLT_MAX, INFINITY };
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		float x = tbl[i];
		float a = f1(x);
		float b = f2(x);
		float e = fabs(a - b);
		printf("x=%e std=%e fmath2=%e diff=%e\n", x, a, b, e);
		a = f1(-x);
		b = f2(-x);
		e = fabs(a - b);
		printf("x=%e std=%e fmath2=%e diff=%e\n", -x, a, b, e);
	}
}

CYBOZU_TEST_AUTO(expLimit)
{
	puts("expLimit");
	limitTest(std::exp, fmath_expf);
	float x = 0.000151307;
	printf("std:  exp=%.8e\n", expf(x));
	printf("fmath:exp=%.8e\n", fmath_expf(x));
}

CYBOZU_TEST_AUTO(tanhLimit)
{
	puts("tanhLimit");
	limitTest(std::tanh, fmath_tanhf);
	float x = 0.000151307;
	printf("std:  tanh=%.8e\n", tanhf(x));
	printf("fmath:tanh=%.8e\n", fmath_tanhf(x));
}
#endif
