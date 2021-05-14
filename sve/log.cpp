#include "log.hpp"
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <vector>
#include <float.h>

float g_maxe = 1e-5;

float diff(float x, float y)
{
	return std::abs(x - y) / x;
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

float logfC(float x)
{
	using namespace fmath;
	const local::ConstVar& C = *local::Inst<>::code.constVar;
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

	const float *tbl = C.logCoeff;
	const int logN = C.logN;
	x = tbl[logN - 1];
	for (int i = logN - 2; i >= 0; i--) {
		x = x * a + tbl[i];
	}
	x = x * a + e;
	return x;
}

void std_log_v(float *dst, const float *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		dst[i] = std::log(src[i]);
	}
}

CYBOZU_TEST_AUTO(log)
{
	float tbl[] = { FLT_MIN, 0.000151307, 0.1, 0.4, 0.5, 0.6, 1 - 1e-4, 1 - 1e-6, 1, 1 + 1e-6, 1 + 1e-4, 100, FLT_MAX, INFINITY };
	const size_t n = CYBOZU_NUM_OF_ARRAY(tbl);
	for (size_t i = 0; i < n; i++) {
		float x = tbl[i];
		float a = log(x);
		float b = fmath::logf(x);
//		float c = logfC(x);
		float e1 = x ? fabs(a - b) / x : fabs(a - b);
//		float e2 = x ? fabs(a - c) / x : fabs(a - c);
//		printf("%zd x=%e a=%e b=%e c=%e e1=%e e2=%e di=%e\n", i, x, a, b, c, e1, e2, fabs(e1 - e2));
		printf("%zd x=%e a=%e b=%e e1=%e\n", i, x, a, b, e1);
		printf("    x=%08x a=%08x b=%08x\n", f2u(x), f2u(a), f2u(b));
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

