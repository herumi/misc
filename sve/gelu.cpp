#include <stdint.h>
#include <math.h>
//#define USE_LOGISTIC
#include "gelu.hpp"
#include <cybozu/test.hpp>
#include <vector>

float g_maxe;

float diff(float x, float y)
{
	return std::abs(x - y) / x;
}

union fi {
	float f;
	uint32_t i;
};

float u2f(uint32_t x)
{
	fi fi;
	fi.i = x;
	return fi.f;
}

uint32_t f2u(float x)
{
	fi fi;
	fi.f = x;
	return fi.i;
}

float std_gelu(float x)
{
#ifdef USE_LOGISTIC
	float y = exp(x);
	return y/(y + 1);
#else
	/*
		0.5x(1 + tanh(sqrt(2/pi)(x + 0.044715x^3)))
		=0.5x(1 + tanh(sqrt(2/pi)(1 + 0.044715x^2)x))
		=0.5x(1 + tanh((A + Bx^2)x))

		C = A + Bx^2
		G = 0.5x(1 + tanh(C))
		= 0.5x(1 + 1 - 2 / (1 + exp(2C)))
		= 0.5x(2 - 2 / (1 + exp(2C)))
		= x(1 - 1/(1+exp(2C)))

		C1 = 2sqrt(2/pi)
		C2 = 2sqrt(2/pi) * 0.044715
		C = C1 + C2 x^2
		G = x(1 - 1/(1 + exp(C x)))
	*/
	static const float C1 = u2f(0x3fcc422a);
	static const float C2 = u2f(0x3d922279);
	float C = C1 + C2 * x * x;
	float y = x*(1-1/(1+exp(C*x)));

#if 0
	float org = 0.5*x*(1 + tanh(sqrt(2/3.14159265358979)*(x + 0.044715 * x * x * x)));
	float e = fabs(y - org);
	if (e > 1e-6) {
		printf("diff %e x=%f y=%f org=%f\n", e, x, y, org);
	}
#endif
	return y;
#endif
}

void std_gelu_v(float *dst, const float *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		dst[i] = std_gelu(src[i]);
	}
}

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

typedef std::vector<float> Fvec;

CYBOZU_TEST_AUTO(bench)
{
	Fvec x, y;
	size_t n = 300;//128;//1024 * 4;
	x.resize(n);
	y.resize(n);
	for (size_t i = 0; i < n; i++) {
		x[i] = (i / float(n) - 0.5) * 4;
x[i] = i - 64;
	}
	fmath::gelu_v(&y[0], &x[0], n);
	for (size_t i = 0; i < n; i++) {
		float y1 = y[i];
		float y2 = std_gelu(x[i]);
		float d = fabs(y1 - y2);
		if (d > 1e-5) {
			printf("x=%e ng=%e ok=%e d=%e\n", x[i], y1, y2, d);
		}
	}
}
