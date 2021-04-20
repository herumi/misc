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
		float a = log(x);
		float b = fmath::logf(x);
		float e = x ? fabs(a - b) / x : fabs(a - b);
		printf("%zd x=%e a=%e b=%e e=%e\n", i, x, a, b, e);
//		CYBOZU_TEST_ASSERT(e < 1e-5);
	}
}
