/*
  see http://en.wikipedia.org/wiki/Fast_inverse_square_root

err 9.692228e-005 max 6.933212e-004
err 2.335245e-002 max 1.025827e-001
err 2.349671e-002 max 1.032121e-001
err 1.578313e-002 max 9.164917e-002
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
	f := int((log(x+1)/log(2)-(x+t))^2,x=0..1);
	s := solve(diff(f,t)=0,t);
	s := evalf(s,15);
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

void test(float begin, float end, float d, float f(float))
{
	double s = 0;
	double m = 0;
	int n = 0;
	for (float x = begin; x < end; x += d) {
		float y = 1 / sqrt(x);
#if 1
		float z = f(x);
#else
		float z = newton(x, f);
#endif
		float ds = fabs(y - z);
		s += ds / y;
		if (ds > m) m = ds;
		n++;
	}
	s /= n;
	printf("err %e max %e\n", s, m);
}

int main()
{
	const float begin = 0.1;
	const float end = 10000;
	const float d = 0.003;
	test(begin, end, d, rsqrt0);
	test(begin, end, d, rsqrt1);
	test(begin, end, d, rsqrt2);
	test(begin, end, d, rsqrt3);
}
