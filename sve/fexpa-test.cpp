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

float u2f(uint32_t u)
{
	fi fi;
	fi.i = u;
	return fi.f;
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
	static const float c2 = 0.5 * log2 * log2;
	float y = x * log2_e;
	int n = (int)floor(y);
	float a = y - n; // 0 <= a < 1
	float b = 1 + a; // 1 <= b < 2, y = (n-1) + b
	uint32_t bu = f2u(b);

	float bL = u2f(bu >> 17);
	float z = b - u2f(bu & ~FexpaTbl::mask(17));
	/*
		split b into bL and z where bL is for fexpa and z is remain
	*/
	float c = fexpaEmu(bL);
	float d = 1 + (log2 + z * c2) * z;
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

