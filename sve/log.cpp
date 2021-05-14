#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <vector>
#include <float.h>
#include <math.h>
#ifdef __x86_64__

#include <limits>

namespace fmath {

namespace local {

union fi {
	uint32_t i;
	float f;
};

struct ConstVar {
	static const size_t logN = 9;
	uint32_t i127shl23;
	uint32_t x7fffff;
	float log2;
	float log1p5;
	float f2div3;
	float fNan;
	float fMInf;
	float logCoeff[logN];
	//
	void init()
	{
		i127shl23 = 127 << 23;
		x7fffff = 0x7fffff;
		log2 = std::log(2.0f);
		log1p5 = std::log(1.5f);
		f2div3 = 2.0f/3;
		fi fi;
		fi.i = 0x7fc00000;
		fNan = fi.f;
		fi.i = 0xff800000;
		fMInf = fi.f;
		const float logTbl[logN] = {
			 1.0, // must be 1
			-0.49999985195974875681242,
			 0.33333220526061677705782,
			-0.25004206220486390058000,
			 0.20010985747510067100077,
			-0.16481566812093889672203,
			 0.13988269735629330763020,
			-0.15049504706005165294002,
			 0.14095711402233803479921,
		};
		for (size_t i = 0; i < logN; i++) {
			logCoeff[i] = logTbl[i];
		}
	}
};

struct Code {
	static ConstVar s_constVar;
	ConstVar *constVar;
	Code()
		: constVar(&s_constVar)
	{
		constVar->init();
	}
};

ConstVar Code::s_constVar;

template<size_t dummy = 0>
struct Inst {
	static const Code code;
};

template<size_t dummy>
alignas(32) const Code Inst<dummy>::code;

} // fmath::local

} // fmath
#else

#include "log.hpp"

#endif

float g_maxe = 1e-5;

float diff(float x, float y)
{
	float d = abs(x - y);
	return x == 0 ? d : d / x;
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
	if (x < 0) return -std::numeric_limits<float>::quiet_NaN();
	if (x == 0) return -std::numeric_limits<float>::infinity();
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

#ifdef __x86_64__
namespace fmath {
void logf_v(float *y, const float *x, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		y[i] = logfC(x[i]);
	}
}

float logf(float x)
{
	return logfC(x);
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
	const float *x = (const float*)xx;
	float y[16];
	fmath::logf_v(y, x, 16);
	for (int i = 0; i < 16; i++) {
		printf("x=%e fmath=%e std=%e\n", x[i], y[i], log(x[i]));
	}
}

CYBOZU_TEST_AUTO(log)
{
	float tbl[] = { FLT_MIN, 0.000151307, 0.1, 0.4, 0.5, 0.6, 1 - 1e-4, 1 - 1e-6, 1, 1 + 1e-6, 1 + 1e-4, 1.000333, 100, FLT_MAX, INFINITY };
	const size_t n = CYBOZU_NUM_OF_ARRAY(tbl);
	for (size_t i = 0; i < n; i++) {
		float x = tbl[i];
		float a = log(x);
		float b = fmath::logf(x);
//		float c = logfC(x);
		float e1 = diff(a, b);
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

#ifndef __x86_64__
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
#endif
