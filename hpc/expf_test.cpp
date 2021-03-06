/*
FCCpx -Kfast -Krestp=all expf_test.cpp on SPARC64 IXfx
fmath_exp aveDiff=      1e-06 maxDiff=3.891789e-06
new_exp   aveDiff=      2e-08 maxDiff=1.192089e-07
std::exp   0.210750
fmath_exp  0.119110
new_exp    0.114230
std::exp4  0.116130
fmath_exp4 0.121790
new_exp4   0.066480
dummy=1213628.375000

g++ -Ofast -march=native expf_test.cpp on Haswell
fmath_exp aveDiff=      1e-06 maxDiff=5.086112e-06
new_exp   aveDiff=      2e-08 maxDiff=1.192089e-07
std::exp   0.024020
fmath_exp  0.014020
new_exp    0.022640
std::exp4  0.091110
fmath_exp4 0.031910
new_exp4   0.071500
dummy=1213674.625000

*/
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef _MSC_VER
	#define RESTRICT
	#include <intrin.h>
	#define MIE_ALIGN(x) __declspec(align(x))
#else
	#define MIE_ALIGN(x) __attribute__((aligned(x)))
	#ifdef __x86_64__
		#include <x86intrin.h>
	#endif
	#ifdef __FUJITSU
		#define RESTRICT
		#include <fjmfunc.h> // for v_exp
		#include <emmintrin.h>
	#else
		#define RESTRICT __restrict__
	#endif
	#include <sys/time.h>
#endif

void std_exp4(float* RESTRICT y, const float* RESTRICT x)
{
#ifdef __FUJITSU
	long n = 4;
	v_exp(y, &n, (float*)x);
#else
	for (int i = 0; i < 4; i++) {
		y[i] = expf(x[i]);
	}
#endif
}

void std_exp4d(double* RESTRICT y, const double* RESTRICT x)
{
	for (int i = 0; i < 4; i++) {
		y[i] = exp(x[i]);
	}
}

void std_exp8d(double* RESTRICT y, const double* RESTRICT x)
{
	for (int i = 0; i < 8; i++) {
		y[i] = (double)exp((float)x[i]);
	}
}

#define FMATH_EXP_TABLE_SIZE 7

const unsigned int kFmathExpTable[128] = {
  0x00000000, 0x0000b1ed, 0x000164d2, 0x000218af,
  0x0002cd87, 0x00038359, 0x00043a29, 0x0004f1f6,
  0x0005aac3, 0x00066491, 0x00071f62, 0x0007db35,
  0x0008980f, 0x000955ee, 0x000a14d5, 0x000ad4c6,
  0x000b95c2, 0x000c57ca, 0x000d1adf, 0x000ddf04,
  0x000ea43a, 0x000f6a81, 0x001031dc, 0x0010fa4d,
  0x0011c3d3, 0x00128e72, 0x00135a2b, 0x001426ff,
  0x0014f4f0, 0x0015c3ff, 0x0016942d, 0x0017657d,
  0x001837f0, 0x00190b88, 0x0019e046, 0x001ab62b,
  0x001b8d3a, 0x001c6573, 0x001d3eda, 0x001e196e,
  0x001ef532, 0x001fd228, 0x0020b051, 0x00218faf,
  0x00227043, 0x0023520f, 0x00243516, 0x00251958,
  0x0025fed7, 0x0026e595, 0x0027cd94, 0x0028b6d5,
  0x0029a15b, 0x002a8d26, 0x002b7a3a, 0x002c6897,
  0x002d583f, 0x002e4934, 0x002f3b79, 0x00302f0e,
  0x003123f6, 0x00321a32, 0x003311c4, 0x00340aaf,
  0x003504f3, 0x00360094, 0x0036fd92, 0x0037fbf0,
  0x0038fbaf, 0x0039fcd2, 0x003aff5b, 0x003c034a,
  0x003d08a4, 0x003e0f68, 0x003f179a, 0x0040213b,
  0x00412c4d, 0x004238d2, 0x004346cd, 0x0044563f,
  0x0045672a, 0x00467991, 0x00478d75, 0x0048a2d8,
  0x0049b9be, 0x004ad226, 0x004bec15, 0x004d078c,
  0x004e248c, 0x004f4319, 0x00506334, 0x005184df,
  0x0052a81e, 0x0053ccf1, 0x0054f35b, 0x00561b5e,
  0x005744fd, 0x00587039, 0x00599d16, 0x005acb94,
  0x005bfbb8, 0x005d2d82, 0x005e60f5, 0x005f9613,
  0x0060ccdf, 0x0062055b, 0x00633f89, 0x00647b6d,
  0x0065b907, 0x0066f85b, 0x0068396a, 0x00697c38,
  0x006ac0c7, 0x006c0719, 0x006d4f30, 0x006e9910,
  0x006fe4ba, 0x00713231, 0x00728177, 0x0073d290,
  0x0075257d, 0x00767a41, 0x0077d0df, 0x0079295a,
  0x007a83b3, 0x007bdfed, 0x007d3e0c, 0x007e9e11
};

inline unsigned int mask(int x)
{
	return (1U << x) - 1;
}

typedef union {
	float f;
	unsigned int i;
} fi;

float fmath_exp(float x)
{
	const int s = FMATH_EXP_TABLE_SIZE;
	const int n = 1 << s;
	const float a0 = n / logf(2.0);
	const float b0 = logf(2.0) / n;
	const float magic = (1 << 23) + (1 << 22); // to round

	float t = x * a0;
	t += magic;
	fi fi;
	fi.f = t;
	t = x - (t - magic) * b0;
	int u = ((fi.i + (127 << s)) >> s) << 23;
	unsigned int v = fi.i & mask(s);
	fi.i = u | kFmathExpTable[v];
	return (1.0f + t) * fi.f;
}

void fmath_exp4(float* RESTRICT y, const float* RESTRICT x)
{
	const int s = FMATH_EXP_TABLE_SIZE;
	const int n = 1 << s;
	const float a0 = n / logf(2.0);
	const float b0 = logf(2.0) / n;
	const float magic = (1 << 23) + (1 << 22); // to round

	float t0 = x[0] * a0;
	float t1 = x[1] * a0;
	float t2 = x[2] * a0;
	float t3 = x[3] * a0;
	t0 += magic;
	t1 += magic;
	t2 += magic;
	t3 += magic;
	fi fi0, fi1, fi2, fi3;
	fi0.f = t0;
	fi1.f = t1;
	fi2.f = t2;
	fi3.f = t3;
	t0 = x[0] - (t0 - magic) * b0;
	t1 = x[1] - (t1 - magic) * b0;
	t2 = x[2] - (t2 - magic) * b0;
	t3 = x[3] - (t3 - magic) * b0;
	int u0 = ((fi0.i + (127 << s)) >> s) << 23;
	int u1 = ((fi1.i + (127 << s)) >> s) << 23;
	int u2 = ((fi2.i + (127 << s)) >> s) << 23;
	int u3 = ((fi3.i + (127 << s)) >> s) << 23;
	unsigned int v0 = fi0.i & mask(s);
	unsigned int v1 = fi1.i & mask(s);
	unsigned int v2 = fi2.i & mask(s);
	unsigned int v3 = fi3.i & mask(s);
	fi0.i = u0 | kFmathExpTable[v0];
	fi1.i = u1 | kFmathExpTable[v1];
	fi2.i = u2 | kFmathExpTable[v2];
	fi3.i = u3 | kFmathExpTable[v3];
	y[0] = (1.0f + t0) * fi0.f;
	y[1] = (1.0f + t1) * fi1.f;
	y[2] = (1.0f + t2) * fi2.f;
	y[3] = (1.0f + t3) * fi3.f;
}

#if 0
// remez(exp(x), 5, [-1/8, 1/8]); by sollya
#define FMATH_EXP_C0 1.00000000016564766
#define FMATH_EXP_C1 1.00000000014198308
#define FMATH_EXP_C2 0.49999980918318004
#define FMATH_EXP_C3 0.16666660911936907
#define FMATH_EXP_C4 0.041699229294277873
#define FMATH_EXP_C5 0.0083395356689759046
#else
#define FMATH_EXP_C0 1
#define FMATH_EXP_C1 1.00000000006177459
#define FMATH_EXP_C2 0.49999988007542528
#define FMATH_EXP_C3 0.16666663108604157
#define FMATH_EXP_C4 0.041694294620381676
#define FMATH_EXP_C5 0.0083383426505236529
#endif
#define FMATH_EXP_DEGREE5(y, t) double y = ((((FMATH_EXP_C5 * t + FMATH_EXP_C4) * t + FMATH_EXP_C3) * t + FMATH_EXP_C2) * t + FMATH_EXP_C1) * t + FMATH_EXP_C0; \
	y *= y; \
	y *= y; \
	y *= y; \
	y *= y; \
	y *= y; \
	y *= y; \
	y *= y; \
	y *= y;

/*
	assume x in [-30, 30]
*/
float new_exp(float x)
{
	// add:5, mul:5
	// mul:8
	// |t| < 1/8
	double t = x / 256;
	FMATH_EXP_DEGREE5(y, t);
	return (float)y;
}

void new_exp4(float py[4], const float px[4])
{
#ifdef __FUJITSU
	const __m128d c256 = _mm_set_pd(1.0 / 256, 1.0 / 256);
	const __m128d c0 = _mm_set_pd(FMATH_EXP_C0, FMATH_EXP_C0);
	const __m128d c1 = _mm_set_pd(FMATH_EXP_C1, FMATH_EXP_C1);
	const __m128d c2 = _mm_set_pd(FMATH_EXP_C2, FMATH_EXP_C2);
	const __m128d c3 = _mm_set_pd(FMATH_EXP_C3, FMATH_EXP_C3);
	const __m128d c4 = _mm_set_pd(FMATH_EXP_C4, FMATH_EXP_C4);
	const __m128d c5 = _mm_set_pd(FMATH_EXP_C5, FMATH_EXP_C5);
	__m128d t0 = _fjsp_set_v2r8(px[1], px[0]);
	__m128d t1 = _fjsp_set_v2r8(px[3], px[2]);
	t0 = _mm_mul_pd(t0, c256);
	t1 = _mm_mul_pd(t1, c256);
	__m128d y0, y1;
	y0 = _fjsp_madd_v2r8(c5, t0, c4);
	y1 = _fjsp_madd_v2r8(c5, t1, c4);

	y0 = _fjsp_madd_v2r8(y0, t0, c3);
	y1 = _fjsp_madd_v2r8(y1, t1, c3);

	y0 = _fjsp_madd_v2r8(y0, t0, c2);
	y1 = _fjsp_madd_v2r8(y1, t1, c2);

	y0 = _fjsp_madd_v2r8(y0, t0, c1);
	y1 = _fjsp_madd_v2r8(y1, t1, c1);

	y0 = _fjsp_madd_v2r8(y0, t0, c0);
	y1 = _fjsp_madd_v2r8(y1, t1, c0);

	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	MIE_ALIGN(16) double out[4];
	_mm_store_pd(out + 0, y0);
	_mm_store_pd(out + 2, y1);
	py[0] = (float)out[0];
	py[1] = (float)out[1];
	py[2] = (float)out[2];
	py[3] = (float)out[3];
#else
	double t0 = px[0] / 256;
	double t1 = px[1] / 256;
	double t2 = px[2] / 256;
	double t3 = px[3] / 256;

	FMATH_EXP_DEGREE5(y0, t0)
	FMATH_EXP_DEGREE5(y1, t1)
	FMATH_EXP_DEGREE5(y2, t2)
	FMATH_EXP_DEGREE5(y3, t3)

	py[0] = (float)y0;
	py[1] = (float)y1;
	py[2] = (float)y2;
	py[3] = (float)y3;
#endif

#if 0
	__m128 xf = _mm_load_ps(px);
	__m128 c256 = _mm_set_ps1(1.0f / 256);
	xf = _mm_mul_ps(xf, c256);
#ifdef __AVX2__
	__m256d t = _mm256_cvtps_pd(xf);
	const __m256d c0 = _mm256_set1_pd(FMATH_EXP_C0);
	const __m256d c1 = _mm256_set1_pd(FMATH_EXP_C1);
	const __m256d c2 = _mm256_set1_pd(FMATH_EXP_C2);
	const __m256d c3 = _mm256_set1_pd(FMATH_EXP_C3);
	const __m256d c4 = _mm256_set1_pd(FMATH_EXP_C4);
	const __m256d c5 = _mm256_set1_pd(FMATH_EXP_C5);
	__m256d y;
	y = _mm256_fmadd_pd(c5, t, c4);
	y = _mm256_fmadd_pd(y, t, c3);
	y = _mm256_fmadd_pd(y, t, c2);
	y = _mm256_fmadd_pd(y, t, c1);
	y = _mm256_fmadd_pd(y, t, c0);

	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	y = _mm256_mul_pd(y, y);
	__m128 yf = _mm256_cvtpd_ps(y);
	_mm_store_ps(py, yf);
#else
	__m128d t0 = _mm_cvtps_pd(xf);
	__m128d t1 = _mm_cvtps_pd(_mm_movehl_ps(xf, xf));
	const __m128d c0 = _mm_set1_pd(FMATH_EXP_C0);
	const __m128d c1 = _mm_set1_pd(FMATH_EXP_C1);
	const __m128d c2 = _mm_set1_pd(FMATH_EXP_C2);
	const __m128d c3 = _mm_set1_pd(FMATH_EXP_C3);
	const __m128d c4 = _mm_set1_pd(FMATH_EXP_C4);
	const __m128d c5 = _mm_set1_pd(FMATH_EXP_C5);
	__m128d y0, y1;
	y0 = _mm_mul_pd(c5, t0); y0 = _mm_add_pd(y0, c4);
	y1 = _mm_mul_pd(c5, t1); y1 = _mm_add_pd(y1, c4);

	y0 = _mm_mul_pd(y0, t0); y0 = _mm_add_pd(y0, c3);
	y1 = _mm_mul_pd(y1, t1); y1 = _mm_add_pd(y1, c3);

	y0 = _mm_mul_pd(y0, t0); y0 = _mm_add_pd(y0, c2);
	y1 = _mm_mul_pd(y1, t1); y1 = _mm_add_pd(y1, c2);

	y0 = _mm_mul_pd(y0, t0); y0 = _mm_add_pd(y0, c1);
	y1 = _mm_mul_pd(y1, t1); y1 = _mm_add_pd(y1, c1);

	y0 = _mm_mul_pd(y0, t0); y0 = _mm_add_pd(y0, c0);
	y1 = _mm_mul_pd(y1, t1); y1 = _mm_add_pd(y1, c0);

	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);

	__m128 y0f = _mm_cvtpd_ps(y0);
	__m128 y1f = _mm_cvtpd_ps(y1);
	y0f = _mm_movelh_ps(y0f, y1f);
	_mm_store_ps(py, y0f);
#endif
#endif
}

#ifdef __FUJITSU
void new_exp4d(double py[4], const double px[4])
{
	const __m128d c256 = _mm_set_pd(1.0 / 256, 1.0 / 256);
	const __m128d c0 = _mm_set_pd(FMATH_EXP_C0, FMATH_EXP_C0);
	const __m128d c1 = _mm_set_pd(FMATH_EXP_C1, FMATH_EXP_C1);
	const __m128d c2 = _mm_set_pd(FMATH_EXP_C2, FMATH_EXP_C2);
	const __m128d c3 = _mm_set_pd(FMATH_EXP_C3, FMATH_EXP_C3);
	const __m128d c4 = _mm_set_pd(FMATH_EXP_C4, FMATH_EXP_C4);
	const __m128d c5 = _mm_set_pd(FMATH_EXP_C5, FMATH_EXP_C5);
	__m128d t0 = _mm_load_pd(px + 0);
	__m128d t1 = _mm_load_pd(px + 2);
	t0 = _mm_mul_pd(t0, c256);
	t1 = _mm_mul_pd(t1, c256);
	__m128d y0, y1;
	y0 = _fjsp_madd_v2r8(c5, t0, c4);
	y1 = _fjsp_madd_v2r8(c5, t1, c4);

	y0 = _fjsp_madd_v2r8(y0, t0, c3);
	y1 = _fjsp_madd_v2r8(y1, t1, c3);

	y0 = _fjsp_madd_v2r8(y0, t0, c2);
	y1 = _fjsp_madd_v2r8(y1, t1, c2);

	y0 = _fjsp_madd_v2r8(y0, t0, c1);
	y1 = _fjsp_madd_v2r8(y1, t1, c1);

	y0 = _fjsp_madd_v2r8(y0, t0, c0);
	y1 = _fjsp_madd_v2r8(y1, t1, c0);

	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	_mm_store_pd(py + 0, y0);
	_mm_store_pd(py + 2, y1);
}

void new_exp8d(double py[8], const double px[8])
{
	const __m128d c256 = _mm_set_pd(1.0 / 256, 1.0 / 256);
	const __m128d c0 = _mm_set_pd(FMATH_EXP_C0, FMATH_EXP_C0);
	const __m128d c1 = _mm_set_pd(FMATH_EXP_C1, FMATH_EXP_C1);
	const __m128d c2 = _mm_set_pd(FMATH_EXP_C2, FMATH_EXP_C2);
	const __m128d c3 = _mm_set_pd(FMATH_EXP_C3, FMATH_EXP_C3);
	const __m128d c4 = _mm_set_pd(FMATH_EXP_C4, FMATH_EXP_C4);
	const __m128d c5 = _mm_set_pd(FMATH_EXP_C5, FMATH_EXP_C5);
	__m128d t0 = _mm_load_pd(px + 0);
	__m128d t1 = _mm_load_pd(px + 2);
	__m128d t2 = _mm_load_pd(px + 4);
	__m128d t3 = _mm_load_pd(px + 6);
	t0 = _mm_mul_pd(t0, c256);
	t1 = _mm_mul_pd(t1, c256);
	t2 = _mm_mul_pd(t2, c256);
	t3 = _mm_mul_pd(t3, c256);
	__m128d y0, y1, y2, y3;
	y0 = _fjsp_madd_v2r8(c5, t0, c4);
	y1 = _fjsp_madd_v2r8(c5, t1, c4);
	y2 = _fjsp_madd_v2r8(c5, t2, c4);
	y3 = _fjsp_madd_v2r8(c5, t3, c4);

	y0 = _fjsp_madd_v2r8(y0, t0, c3);
	y1 = _fjsp_madd_v2r8(y1, t1, c3);
	y2 = _fjsp_madd_v2r8(y2, t2, c3);
	y3 = _fjsp_madd_v2r8(y3, t3, c3);

	y0 = _fjsp_madd_v2r8(y0, t0, c2);
	y1 = _fjsp_madd_v2r8(y1, t1, c2);
	y2 = _fjsp_madd_v2r8(y2, t2, c2);
	y3 = _fjsp_madd_v2r8(y3, t3, c2);

	y0 = _fjsp_madd_v2r8(y0, t0, c1);
	y1 = _fjsp_madd_v2r8(y1, t1, c1);
	y2 = _fjsp_madd_v2r8(y2, t2, c1);
	y3 = _fjsp_madd_v2r8(y3, t3, c1);

	y0 = _fjsp_madd_v2r8(y0, t0, c0);
	y1 = _fjsp_madd_v2r8(y1, t1, c0);
	y2 = _fjsp_madd_v2r8(y2, t2, c0);
	y3 = _fjsp_madd_v2r8(y3, t3, c0);

	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);
	y0 = _mm_mul_pd(y0, y0); y1 = _mm_mul_pd(y1, y1);

	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);
	y2 = _mm_mul_pd(y2, y2); y3 = _mm_mul_pd(y3, y3);

	_mm_store_pd(py + 0, y0);
	_mm_store_pd(py + 2, y1);
	_mm_store_pd(py + 4, y2);
	_mm_store_pd(py + 6, y3);
}
#endif

uint64_t getClk()
{
#ifdef _MSC_VER
	return __rdtsc();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}
float bench1(const char *msg, float func(float))
{
	uint64_t clk = getClk();
	float x = 1.2f;
	float t = 0;
	const int N = 100000;
	for (int i = 0; i < N; i++) {
		t += func(x);
		x += 1e-7;
	}
	double time = (getClk() - clk) / double(N);
	printf("%s %f\n", msg, time);
	return t * x;
}

void test(const char *msg, float func(float), float begin, float end, float step)
{
	double maxDiff = 0;
	double sumDiff = 0;
	int count = 0;
	for (float x = begin; x < end; x += step) {
		float a = exp(x);
		float b = func(x);
		double d = fabs(a - b) / (double)a;
		sumDiff += d;
		count++;
		if (d > maxDiff) {
			maxDiff = d;
		}
	}
	printf("%s aveDiff=%11.e maxDiff=%11.6e\n", msg, sumDiff / count, maxDiff);
}

template<size_t N, class T>
void testv(const char *msg, void func(T*, const T *), float begin, float end, float step)
{
	double maxDiff = 0;
	double sumDiff = 0;
	int count = 0;
	MIE_ALIGN(16) T a[N];
	MIE_ALIGN(16) T b[N];
	for (double x = begin; x < end; x += step * N) {
		for (int i = 0; i < N; i++) {
			a[i] = T(x + step * i);
		}
		func(b, a);
		for (int i = 0; i < N; i++) {
			double aa = exp(a[i]);
			double d = fabs(b[i] - aa) / aa;
			sumDiff += d;
			count++;
			if (d > maxDiff) {
				maxDiff = d;
			}
		}
	}
	printf("%s aveDiff=%11.e maxDiff=%11.6e\n", msg, sumDiff / count, maxDiff);
}

template<size_t N, class T>
float benchv(const char *msg, void func(T*, const T *))
{
	MIE_ALIGN(16) T xa[N];
	MIE_ALIGN(16) T ya[N];
	for (size_t i = 0; i < N; i++) {
		xa[i] = (T)sin(double(i)) * 3;
	}
	uint64_t clk = getClk();
	const int C = 100000;
	for (int j = 0; j < C; j++) {
		func(ya, xa);
		for (size_t i = 0; i < N; i++) {
			xa[i] += 1e-7;
		}
	}
	double time = (getClk() - clk) / double(N);
	double t = 0;
	for (size_t i = 0; i < N; i++) {
		t += ya[i];
	}
	printf("%s %8.3f\n", msg, time);
	return t;
}

int main()
{
	const float begin = -30.0f;
	const float end = 30.0f;
	const float step = 1e-4f;
	test("fmath_exp", fmath_exp, begin, end, step);
	test("new_exp  ", new_exp, begin, end, step);
	testv<4>("new_exp4 ", new_exp4, begin, end, step);
	testv<8>("std_exp8d", std_exp8d, begin, end, step);
#ifdef __FUJITSU
	testv<4>("new_exp4d", new_exp4d, begin, end, step);
	testv<8>("new_exp8d", new_exp8d, begin, end, step);
#endif
	float dummy = 0;
	dummy += bench1("std::exp  ", expf);
	dummy += bench1("fmath_exp ", fmath_exp);
	dummy += bench1("new_exp   ", new_exp);

	puts("---");
	dummy += benchv<4>("std::exp4 ", std_exp4);
	dummy += benchv<4>("fmath_exp4", fmath_exp4);
	dummy += benchv<4>("new_exp4  ", new_exp4);
#ifdef __FUJITSU
	dummy += benchv<4>("new_exp4d ", new_exp4d);
#endif

	puts("---");
	dummy += benchv<8>("std_exp8d ", std_exp8d);
#ifdef __FUJITSU
	dummy += benchv<8>("new_exp8d ", new_exp8d);
#endif
	printf("dummy=%f\n", dummy); // avoid optimization
}
