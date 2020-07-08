#include <stdio.h>
#include "fexpa.hpp"

union fi {
	float f;
	uint32_t i;
};

uint32_t f2u(float x)
{
	fi fi;
	fi.f = x;
	return fi.i;
}

float diff(float x, float y)
{
	return std::abs(x - y) / x;
}

template<class F>
float putDiff(float begin, float end, float step, const F& f)
{
	float maxe = 0;
	float maxx = 0;
	double ave = 0;
	int aveN = 0;
	for (float x = begin; x < end; x += step) {
		float y0 = std::exp(x);
		float y1 = f(x);
		float e;
		e = diff(y0, y1);
		if (e > maxe) {
			maxe = e;
			maxx = x;
		}
		ave += e;
		aveN++;
	}
	printf("range [%.2e, %.2e] step=%.2e\n", begin, end, step);
	printf("maxe=%e (x=%e)\n", maxe, maxx);
	printf("ave=%e\n", ave / aveN);
	return maxe;
}

float expC2(float x)
{
	static const float log2 = (float)log(2.0);
	static const float log2_e = float(1 / log2);
	float y = x * log2_e;
	int n = (int)floor(y);
	float a = y - n; // 0 <= a < 1
	float b = 1 + a; // 1 <= b < 2, y = (n-1) + b
	fi m, fi;
	fi.f = b;
	/*
		b.i = (127 << 23) + mantissa where len(mantissa) = 23
		mantissa = (L << 17) | R where len(L) = 6, len(R) = 17
		mantissa/2^24 = (L/2^7) + R/2^24
	*/
	m.i = fi.i & ~FexpaTbl::mask(17);
	fi.i >>= 17;
	float c = fexpaEmu(fi.f);
	float z = b - m.f;
	const float c0 = 1;
	const float c1 = log2;
	const float c2 = 0.5 * log2 * log2;
	float d = c0 + (c1 + z * c2) * z;
	return powf(2, (float)n) * c * d;
}

int main()
{
	for (float x = 0; x < 2; x += 0.1) {
		float y1 = expf(x);
		float y2 = expC2(x);
		float e = fabsf(y1 - y2);
		printf("x=%f, y1=%f, y2=%f, diff=%e\n", x, y1, y2, e);
	}
	puts("expC2");
	putDiff(-10, 10, 0.5, expC2);
	putDiff(-30, 30, 1e-5, expC2);
}

