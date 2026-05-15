#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

// return pow(x, n) % m
// assume 0 <= x && 0 < m < 32768
constexpr int powMod(int x, int n, int m)
{
	int64_t p_inv = x;
	int64_t r = 1;
	while (n > 0) {
		if (n & 1) {
			r = (r * p_inv) % m;
		}
		p_inv = (p_inv * p_inv) % m;
		n >>= 1;
	}
	return int(r);
}

const int p = 3329;
const int shift = 16;
const int R = 1<< shift;
const int R_inv = powMod(R, p-2, p); // R^(-1) % p=169
const int p_inv = (1 - R * R_inv)/p; // p_inv^(-1) % R = (1 - R * R_inv)/p=-3327
const int RR = ((R%p)*(R%p))%p;
const int M0 = 32767;
const int M1 = 32767;//p*9;

int maskL(int x) { return x & (R-1); }

int psubw(int x, int y) {
	return short(x - y);
}

int pmullw(int x, int y) {
	// avoid overflow
	return maskL(int64_t(x) * y);
//	return maskL(x * y);
}

int pmulhw(int x, int y) {
	return (x * y) >> shift;
}

int vpmulhrsw(int x, int y) {
	int prod = (x * y) + (R/4);
	return prod >> 15;
}
// AArch64

int sqrdmulh(int x, int y) {
	return vpmulhrsw(x, y);
}

int mls(int acc, int x, int y) {
	return psubw(acc, pmullw(x, y));
}

int modp(int x) {
	int r = x % p;
	if (r < 0) r += p;
	return r;
}

void putParam()
{
	printf("p=%d\n", p);
	printf("R=%d\n", R);
	printf("R_inv=%d\n", R_inv);
	printf("p_inv=%d\n", p_inv);
	printf("RR=%d\n", RR);
}

int MR(int x) { // MR(x) = x R'
//	int m = (x * int64_t(p_inv)) & (R-1);
	int m = pmullw(x, p_inv);
	int r = (x - m * p)>>shift;
	if (r >= p) r -= p;
	if (r < 0) r += p;
	return r;
}

int mont(int x, int y) {
	return MR(x * y);
}

int fromMont(int aR) { // mont(aR, 1) = MR(aR) = a
	return mont(aR, 1);
}

int toMont(int a) { // mont(a, RR) = MR(aRR) = aR
	return mont(a, RR);
}

// z = pmullw(y, p_inv)
// return mont(x, y)
int mont1(int x, int y, int z) {
	int t1 = pmullw(x, z);
	int t2 = pmulhw(t1, p);
	int t3 = pmulhw(x, y);
	int r = psubw(t3, t2);
	return r;
}

// Faster AVX2 optimized NTT multiplication for Ring-LWE lattice cryptography
// 2018-039.pdf
// z = (y * (R/2)) / p
// return (x * y) % p
int modp1(int x, int y, int z) {
	int t1 = vpmulhrsw(x, z);
	int t2 = pmullw(t1, p);
	int t3 = pmullw(x, y);
	int r = psubw(t3, t2);
	return r;
}

int modp1_aarch64(int x, int y, int z) {
	int t1 = sqrdmulh(x, z);
	int t2 = pmullw(x, y);
	return mls(t2, t1, p);
}

struct Range {
	static const int X = 0x7fffffff;
	int min = X;
	int max = -X;
	void update(int v)
	{
		if (v < min) min = v;
		if (v > max) max = v;
	}
	void put(const char *msg=nullptr) const
	{
		if (msg) printf("%s ", msg);
		printf("[%d, %d], hex [-%x, %x]\n", min, max, (-min), max);
	}
	void clear()
	{
		min = X;
		max = -X;
	}
};

void toMontTest()
{
	puts("toMontTest");
	for (int a = -M0; a < M0; a++) {
		int aR = toMont(a);
		if ((aR - (a * R)) % p != 0) {
			printf("ERR0 a=%d aR=%d\n", a, aR);
			exit(1);
		}
		int b = fromMont(aR);
		int r = modp(a);
		if (r != b) {
			printf("ERR1 a=%d r=%d b=%d\n", a, r, b);
			exit(1);
		}
	}
	puts("ok");
}

void montTest()
{
	puts("montTest");
#pragma omp parallel for
	for (int a = -M0; a < M0; a++) {
		int aR = toMont(a);
		for (int b = -M0; b < M0; b++) {
			int bR = toMont(b);
			int cR = mont(aR, bR);
			int c = fromMont(cR);
			int ok = modp(a * b);
			if (c != ok) {
				printf("ERR2 a=%d b=%d\n", a, b);
				printf("c=%d, (a * b) modp p = %d\n", c, ok);
				exit(1);
			}
		}
	}
	puts("ok");
}

void mont1Test()
{
	puts("mont1Test");
//#pragma omp parallel for
	Range range;
	// a: coeff of NTT, b:var
	printf("a in [%d, %d]\n", -M1, M1);
	printf("b in [%d, %d]\n", -M0, M0);
	for (int a = -M1; a < M1; a++) {
		int z = pmullw(a, p_inv);
		for (int b = -M0; b < M0; b++) {
			int y = mont1(b, a, z);
			range.update(y);
			int w = mont(a, b);
			if ((y - w) % p) {
				printf("ERR3 a=%d b=%d\n", a, b);
				printf("y = %d, w = %d\n", y, w);
				exit(1);
			}
		}
	}
	range.put("y");
	puts("ok");
}

void modp1Test()
{
	puts("modp1Test");
//#pragma omp parallel for
	Range range;
	// a: coeff of NTT, b: var
	printf("a in [%d, %d]\n", -M1, M1);
	printf("b in [%d, %d]\n", -M0, M0);
	for (int a = -M1; a < M1; a++) {
		int z = (a * (R/2)) / p;
		for (int b = -M0; b < M0; b++) {
			int y = modp1(b, a, z);
			range.update(y);
			int w = modp(a * b);
			if ((y - w) % p) {
				printf("ERR4 a=%d b=%d\n", a, b);
				printf("y = %d, w = %d\n", y, w);
				exit(1);
			}
		}
	}
	range.put("y");
	puts("ok");
}


/*
	Improved Plantard Arithmetic for Lattice-based Cryptography 2022-956.pdf
	Algorithm 10:
	x: variable (a)
	y: constant (b) - not used
	z: precomputed 32-bit constant: (b * p_inv_32) mod 2^32
*/
int modp_plantard(int x, int /*y*/, uint32_t z) {
	const int alpha = 3;
	// 1. [[abq']_{2l}]^l
	// get low 32-bit value of multiplication x (16bit) and z (32bit)
	uint32_t t1 = (uint32_t)x * z;

	// arithmetic shift as signed integer
	int t2 = (int)t1 >> shift;

	// 2. [(t2 + 2^alpha) * q]^l
	int t3 = t2 + (1 << alpha);

	// multiply by q (i.e. p) and arithmetic right shift by 16 bits
	int r = (t3 * p) >> shift;

	return r;
}

// Improved Plantard Arithmetic for Lattice-based Cryptography
// 2022-956.pdf
// for Cortex-M4
void modp_plantardTest()
{
	puts("modp_plantardTest");
	// inv = pow(p, -1, 2**32)
	const uint32_t inv = 1806234369;
//#pragma omp parallel for
	Range range;
	// a: coeff of NTT, b: var
	printf("a in [%d, %d]\n", -M1, M1);
	printf("b in [%d, %d]\n", -M0, M0);
	for (int a = -M1; a < M1; a++) {
		uint32_t z = uint32_t(a) * inv;
		for (int b = -M0; b < M0; b++) {
			int y = modp_plantard(b, a, z);
			range.update(y);
			int w = modp(a * b);
			int y_org = modp(-y * RR);
			if ((y_org - w) % p) {
				printf("ERR5 a=%d b=%d\n", a, b);
				printf("y_org = %d, w = %d\n", y_org, w);
				exit(1);
			}
		}
	}
	range.put("y");
	puts("ok");
}

int main()
{
//	printf("R_inv=%d\n", powMod(R, p-2,p));
//	printf("R=%d\n", (1-R*R_inv)/p);
	printf("p_inv=%u\n", p_inv);
	putParam();
	toMontTest();
	montTest();
	mont1Test();
	modp1Test();
	modp_plantardTest();
}
