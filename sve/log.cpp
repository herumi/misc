#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <vector>
#include <float.h>
#include <math.h>
#include <limits>
#include "log.hpp"

typedef std::vector<float> floatVec;

const float MAX_E = 1e-6f;

const float g_maxe = 1e-6;

float diff(float x, float y)
{
	float d = abs(x - y);
	return x == 0 ? d : d / x;
}

using namespace fmath::local;

void putf(const char *msg, float x)
{
	printf("%s x=%f(%08x)\n", msg, x, f2u(x));
}

float logfC(float x)
{
	if (x < 0) return -std::numeric_limits<float>::quiet_NaN();
	if (x == 0) return -std::numeric_limits<float>::infinity();
	using namespace fmath;
	const local_log::ConstVar& C = *local_log::Inst<>::code.constVar;
	local::fi fi;
	fi.f = x;
	float e = int(fi.i - (127 << 23)) >> 23;
	fi.i = (fi.i & 0x7fffff) | (127 << 23);
	float y = fi.f;
	/*
		x = y * 2^e (1 <= y < 2)
		log(x) = e log2 + log(y)
		a = (2/3) y - 1 (|a|<=1/3)
		y = 1.5(1 + a)
		log(y) = log 1.5 + log(1 + a)
		log(x) = e log2 + log 1.5 + (a - a^2/2 + a^3/3 - ...)
	*/
	float a = C.f2div3 * y - C.logCoeff[0];
	e = e * C.log2 + C.log1p5;
	float d = x - 1;
	if (fabs(d) <= 1.0/8) {
		a = d;
		e = 0;
	}

	const float *tbl = C.logCoeff;
	const int logN = C.logN;
	x = tbl[logN - 1];
	for (int i = logN - 2; i >= 0; i--) {
		x = x * a + tbl[i];
	}
	x = x * a + e;
	return x;
}

#define NOT_NEAR

float logfC2(float x)
{
	if (x < 0) return -std::numeric_limits<float>::quiet_NaN();
	if (x == 0) return -std::numeric_limits<float>::infinity();
	using namespace fmath;
	const local_log::ConstVar& C = *local_log::Inst<>::code.constVar;
	/*
		x = 2^n a (1 <= a < 2)
		log x = n * log2 + log a
		L = 4
		d = (f2u(a) & mask(23)) >> (23 - L)
		b = T1[d] = approximate of 1/a
		log b = T2[d]
		c = ab - 1 is near zero
		a = (1 + c) / b
		log a = log(1 + c) - log b
	*/
	const uint32_t mask23 = (1 << 23) - 1;
	const uint32_t L = C.L;
	local::fi fi;
	fi.f = x;
	int n = int(fi.i - (127 << 23)) >> 23;
	uint32_t mantissa = fi.i & mask23;
	float a = u2f(mantissa | (127 << 23));
	int d = mantissa >> (23 - L);
	float b = C.tbl1[d];
	float log_b = C.tbl2[d];
	float c = a * b - 1;
	float z = n * C.log2 - log_b;
	if (0.98 <= x && x <= 1.005) {
		c = x - 1;
		z = 0;
	}
	float y = c * (-.250831127) + (.333942362961);
	y = y * c + (-.49999909725);
	y = y * c + 1;
	y = y * c + z;
	return y;
}

float log_near1(float x)
{
	float c = x - 1;
	float z = 0;
	float y = c * (-.250831127) + (.333942362961);
	y = y * c + (-.49999909725);
	y = y * c + 1;
	y = y * c + z;
	return y;
}

#if 0
CYBOZU_TEST_AUTO(sss)
{
	float e = 1e-3;
	for (float x = 1 - 1e-5 - e * 100; x <= 1 + e * 100; x += 1e-4) {
		float a = std::log(x);
		float b = logfC2(x);
		float c = log_near1(x);
		float d1 = fabs(a - b);
		float d2 = fabs(a - c);
		if (a != 0) { d1 /= fabs(a); d2 /= fabs(a); }
//		if (d2 < d1) {
			printf("x=%e log=%e d1=%e d2=%e %e %c\n", x, a, d1, d2, fabs(b-c), d2 < d1 ? 'o' : 'x');
//		}
	}
	exit(1);
}
#endif

#ifdef FMATH_X64_EMU
namespace fmath {
void logf_v(float *y, const float *x, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		y[i] = logfC2(x[i]);
	}
}

float logf(float x)
{
	return logfC2(x);
}
}
#endif

void std_log_v(float *dst, const float *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		dst[i] = std::log(src[i]);
	}
}

CYBOZU_TEST_AUTO(aaa)
{
	uint32_t xx[] = {
		0x0, 0xc0c00000, 0x3db269e2, 0xbd3dd738, 0x42200000, 0xc1a00000,
		0x3da50df1, 0xbf06254e, 0x42b110c2, 0x0, 0x0, 0x40a00000, 0xc1100000,
		0x3d85daf8, 0xbcdb0c30, 0x42200000
	};
	const size_t n = CYBOZU_NUM_OF_ARRAY(xx);
	float x[n];
	for (size_t i = 0; i < n; i++) {
		fmath::local::fi fi;
		fi.i = xx[i];
		x[i] = fi.f;
	}
	float y[16];
	fmath::logf_v(y, x, 16);
	for (int i = 0; i < 16; i++) {
		float a = std::log(x[i]);
//		float a = logfC2(x[i]);
		float b = y[i];
		float e = diff(a, b);
		if (e > g_maxe) {
			printf("%d x=%e std=%e fmath=%e e=%e\n", i, x[i], a, b, e);
		}
	}
}

void checkRange(float (*f)(float), void (*g)(float *, const float *, size_t), float begin, float end, float step)
{
	floatVec dst, src;
	float maxe = -1;
	float maxx = -1;
	double ave = 0;
	int aveN = 0;
	size_t n = size_t((end - begin) / step);
	src.resize(n);
	dst.resize(n);
	{
		float x = begin;
		for (size_t i = 0; i < n; i++) {
			src[i] = x;
			x += step;
		}
	}
	g(&dst[0], &src[0], n);

	for (size_t i = 0; i < n; i++) {
		float x = src[i];
		float y0 = f(x);
		float y1 = dst[i];
		float e;
		e = diff(y0, y1);
		if (!(e <= MAX_E)) {
			printf("err x=%e y0=%e y1=%e e=%e\n", x, y0, y1, e);
		}
		if (e > maxe) {
			maxe = e;
			maxx = x;
		}
		ave += e;
		aveN++;
	}
	ave /= aveN;
	printf("range [%e, %e] step=%f\n", begin, end, step);
	printf("maxe=%e (x=%e)\n", maxe, maxx);
	printf("ave=%e\n", ave);
	CYBOZU_TEST_ASSERT(ave <= MAX_E);
	CYBOZU_TEST_ASSERT(maxe <= MAX_E);
}

CYBOZU_TEST_AUTO(err)
{
	checkRange(std::log, fmath::logf_v, FLT_MIN, 1, 1e-4);
	checkRange(std::log, fmath::logf_v, 1 - 1e-5, 1 + 1e-5, 1e-7);
	checkRange(std::log, fmath::logf_v, 10, 11, 1e-4);
	checkRange(std::log, fmath::logf_v, 1000, 1000 + 1, 1e-4);
}

CYBOZU_TEST_AUTO(log)
{
	float tbl[] = { FLT_MIN, 0.000151307, 0.1, 0.4, 0.5, 0.6, 1 - 1.0/8, 1 - 1e-4, 1 - 1e-6, 1, 1 + 1e-6, 1 + 1e-4, 1.000333, 1 + 1.0/8, 100, FLT_MAX, INFINITY };
	const size_t n = CYBOZU_NUM_OF_ARRAY(tbl);
	for (size_t i = 0; i < n; i++) {
		float x = tbl[i];
		float a = std::log(x);
//		float a = logfC2(x);
		float b = fmath::logf(x);
//		float b = logfC(x);
		float e = diff(a, b);
#if 0
		printf("x=%e(%08x) a=%e(%08x) b=%e(%08x) e=%e\n", x, f2u(x), a, f2u(a), b, f2u(b), e);
#else
		if (e > g_maxe) {
			printf("%zd x=%e a=%e b=%e e=%e\n", i, x, a, b, e);
			printf("    x=%08x a=%08x b=%08x\n", f2u(x), f2u(a), f2u(b));
		}
#endif
//		CYBOZU_TEST_ASSERT(e < 1e-5);
	}
}

void checkDiff(const float *x, const float *y1, const float *y2, size_t n, bool put = true)
{
	for (size_t i = 0; i < n; i++) {
		float d = diff(y1[i], y2[i]);
		if (put) {
			if (!(d <= g_maxe)) {
				printf("err n=%zd, i=%zd x=%e y1=%e y2=%e\n", n, i, x[i], y1[i], y2[i]);
				exit(1);
			}
		} else {
			CYBOZU_TEST_ASSERT(d <= g_maxe);
		}
	}
}

CYBOZU_TEST_AUTO(logf_v)
{
	const size_t n = 300;
	float x[n];
	float y1[n];
	float y2[n];
	for (size_t i = 0; i < n; i++) {
		x[i] = float(i) * 100 + 1e-5;
	}
	std_log_v(y1, x, n);
	fmath::logf_v(y2, x, n);
	checkDiff(x, y1, y2, n);
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
		x[i] = (i + 1) / (C * 0.1);
	}
	printf("for float x[%zd];\n", n);
	CYBOZU_BENCH_C("", C, std_log_v, &y0[0], &x[0], n);
	putClk("std::log", C * (n / 16));
	for (int i = 0; i < 100; i++) {
		memset(&y1[0], 0, i * sizeof(float));
		fmath::logf_v(&y1[0], &x[0], i);
		checkDiff(&x[0], &y0[0], &y1[0], i);
	}
	CYBOZU_BENCH_C("", C, fmath::logf_v, &y1[0], &x[0], n);
	putClk("fmath::logf_v", C * (n / 16));
	checkDiff(&x[0], &y0[0], &y1[0], n);
	n = 1024 * 4;
	memset(y1.data(), 0, n * sizeof(float));
	CYBOZU_BENCH_C("", C, fmath::logf_v, &y1[0], &x[0], n);
	putClk("fmath::logf_v", C * (n / 16));
	checkDiff(&x[0], &y0[0], &y1[0], n);
}
