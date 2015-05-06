/**
	@brief fast math library for float
	@author herumi
	@url http://homepage1.nifty.com/herumi/
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include "fmath.hpp"

#include <math.h>
#include <stddef.h>
#include <assert.h>
#include <limits>
#include <stdlib.h>
#include <float.h>
#include <algorithm>

#ifdef _MSC_VER
#define MIE_ALIGN(x) __declspec(align(x))
#else
#define MIE_ALIGN(x) __attribute__((aligned(x)))
#endif

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

/*
	exp(88.722839f) = inf ; 0x42b17218
	exp(-87.33655f) = 1.175491e-038f(007fffe6) denormal ; 0xc2aeac50
	exp(-103.972081f) = 0 ; 0xc2cff1b5
*/
template<size_t N = EXP_TABLE_SIZE>
struct ExpVar {
	enum {
		s = N,
		n = 1 << s,
		f88 = 0x42b00000 /* 88.0 */
	};
	float minX[8];
	float maxX[8];
	float a[8];
	float b[8];
	float f1[8];
	unsigned int i127s[8];
	unsigned int mask_s[8];
	unsigned int i7fffffff[8];
	unsigned int tbl[n];
	ExpVar()
	{
		float log_2 = ::logf(2.0f);
		for (int i = 0; i < 8; i++) {
			maxX[i] = 88;
			minX[i] = -88;
			a[i] = n / log_2;
			b[i] = log_2 / n;
			f1[i] = 1.0f;
			i127s[i] = 127 << s;
			i7fffffff[i] = 0x7fffffff;
			mask_s[i] = mask(s);
		}

		for (int i = 0; i < n; i++) {
			float y = pow(2.0f, (float)i / n);
			fi fi;
			fi.f = y;
			tbl[i] = fi.i & mask(23);
		}
	}
};

template<size_t sbit_ = EXPD_TABLE_SIZE>
struct ExpdVar {
	enum {
		sbit = sbit_,
		s = 1UL << sbit,
		adj = (1UL << (sbit + 10)) - (1UL << sbit)
	};
	// A = 1, B = 1, C = 1/2, D = 1/6
	double C1[2]; // A
	double C2[2]; // D
	double C3[2]; // C/D
	uint64_t tbl[s];
	double a;
	double ra;
	di inf;
	void init()
	{
		a = s / ::log(2.0);
		ra = 1 / a;
		for (int i = 0; i < 2; i++) {
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
		inf.i = uint64_t(0x7ff0000000000000ull);
	}
};

static ExpdVar<EXPD_TABLE_SIZE> expdVar;
//static MIE_ALIGN(32) const ExpVar<EXP_TABLE_SIZE> expVar;

} // fmath::local

void init()
{
	local::expdVar.init();
}
#if 0
float exp(float x)
{
	using namespace local;

	x = std::min(x, expVar.maxX[0]);
	x = std::max(x, expVar.minX[0]);
	float t = x * expVar.a[0];
	const float magic = (1 << 23) + (1 << 22); // to round
	t += magic;
	fi fi;
	fi.f = t;
	t = x - (t - magic) * expVar.b[0];
	int u = ((fi.i + (127 << expVar.s)) >> expVar.s) << 23;
	unsigned int v = fi.i & mask(expVar.s);
	fi.i = u | expVar.tbl[v];
	return (1 + t) * fi.f;
//	return (1 + t) * pow(2, (float)u) * pow(2, (float)v / n);
}
#endif

double exp(double x)
{
	using namespace local;
	const ExpdVar<>& c = expdVar;
	if (x <= -708.39641853226408) return 0;
	if (x >= 709.78271289338397) return c.inf.d;
/*
	remark : -ffast-math option of gcc may generate bad code for fmath::expd
*/
	const double C1 = 1.0;
	const double C2 = 0.16666666685227835064;
	const double C3 = 3.0000000027955394;
	const uint64_t b = 3ULL << 51;
	di di;
	di.d = x * c.a + b;
	uint64_t iax = c.tbl[di.i & mask(c.sbit)];

	double t = (di.d - b) * c.ra - x;
	uint64_t u = ((di.i + c.adj) >> c.sbit) << 52;
	double y = (C3 - t) * (t * t) * C2 - t + C1;

	di.i = u | iax;
	return y * di.d;
}

}
