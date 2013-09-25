/*
  see http://en.wikipedia.org/wiki/Fast_inverse_square_root
*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <memory.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

float rsqrt0(float x)
{
	__m128 a = _mm_set_ss(x);
	a = _mm_rsqrt_ss(a);
	return _mm_cvtss_f32(a);
}

float rsqrt1(float x)
{
	uint32_t i;
	memcpy(&i, &x, sizeof(i));
	i = 0x5f3759df - (i >> 1);
	memcpy(&x, &i, sizeof(x));
	return x;
}

/*
	http://shelfflag.com/rsqrt.pdf
*/
float rsqrt2(float x)
{
	uint32_t i;
	memcpy(&i, &x, sizeof(i));
	i = 0x5f37642f - (i >> 1);
	memcpy(&x, &i, sizeof(x));
	return x;
}

/*
	f := int((log(x+1)/log(2)-(x+A))^2,x=0..1);
	sol := solve(diff(f,A)=0,A);
	s := evalf(sol,15);
	hex(int(3/2. * (127 - s) * (1 << 23))) = 0x5f34ff58
*/
float rsqrt3(float x)
{
	uint32_t i;
	memcpy(&i, &x, sizeof(i));
	i = 0x5f34ff58 - (i >> 1);
	memcpy(&x, &i, sizeof(x));
	return x;
}

float newton(float x, float rsqrt(float))
{
	float x2 = x * 0.5f;
	x = rsqrt(x);
	return x * (1.5f - x2 * x * x);
}

int main()
{
	double sa = 0, ma = 0;
	double sb = 0, mb = 0;
	double sc = 0, mc = 0;
	double sd = 0, md = 0;
	int n = 0;
	for (float x = 0.1; x < 10000; x += 0.003) {
		float y = 1 / sqrt(x);
		float a = rsqrt0(x);
#if 0
		float b = newton(x, rsqrt1);
		float c = newton(x, rsqrt2);
		float d = newton(x, rsqrt3);
#else
		float b = rsqrt1(x);
		float c = rsqrt2(x);
		float d = rsqrt3(x);
#endif
		float da = fabs(a - y);
		float db = fabs(b - y);
		float dc = fabs(c - y);
		float dd = fabs(d - y);
		sa += da / a;
		sb += db / a;
		sc += dc / a;
		sd += dd / a;
		if (da > ma) ma = da;
		if (db > mb) mb = db;
		if (dc > mc) mc = dc;
		if (dd > md) md = dd;
		n++;
	}
	sa /= n;
	sb /= n;
	sc /= n;
	sd /= n;
	printf("s:%e %e %e %e\n", sa, sb, sc, sd);
	printf("m:%e %e %e %e\n", ma, mb, mc, md);
}
