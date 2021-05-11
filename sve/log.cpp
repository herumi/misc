#include "log.hpp"
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <vector>
#include <float.h>

float g_maxe;

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
	float tbl[] = { 0, 0.000151307, 0.1, 0.4, 0.5, 0.6, 1, 100, INFINITY };
	const size_t n = CYBOZU_NUM_OF_ARRAY(tbl);
	for (size_t i = 0; i < n; i++) {
		float x = tbl[i];
//		float a = log(x);
		float a = logfC(x);
		float b = fmath::logf(x);
		float e = x ? fabs(a - b) / x : fabs(a - b);
		printf("%zd x=%e a=%e b=%e e=%e\n", i, x, a, b, e);
		printf("    x=%08x a=%08x b=%08x\n", f2u(x), f2u(a), f2u(b));
//		CYBOZU_TEST_ASSERT(e < 1e-5);
	}
}
