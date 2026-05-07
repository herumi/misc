#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

// return pow(x, n) % m
// assume 0 <= x && 0 < m < 32768
constexpr int powMod(int x, int n, int m)
{
	int64_t q = x;
	int64_t r = 1;
	while (n > 0) {
		if (n & 1) {
			r = (r * q) % m;
		}
		q = (q * q) % m;
		n >>= 1;
	}
	return int(r);
}

const int p = 3329;
const int shift = 16;
const int R = 1<< shift;
const int Q = powMod(R, p-2, p); // R^(-1) % p=169
const int q = (1 - R * Q)/p; // q^(-1) % R = (1 - R * Q)/p=-3327
const int RR = ((R%p)*(R%p))%p;
const int M = 32767;//p*9;

void putParam()
{
	printf("p=%d\n", p);
	printf("R=%d\n", R);
	printf("Q=%d\n", Q);
	printf("q=%d\n", q);
	printf("RR=%d\n", RR);
}

int MR(int x) { // MR(x) = x R'
    int m = (x * int64_t(q)) & (R-1);
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

int modp(int x) {
	int r = x % p;
	if (r < 0) r += p;
	return r;
}

int mull(int x, int y) {
	return (x * y) & (R - 1);
}

int mulh(int x, int y) {
	return (x * y) >> shift;
}

int mont1(int x, int y, int y_aux) {
	int t1 = mull(x, y_aux);
	int t2 = mulh(t1, p);
	int h = mulh(x, y);
	int r = h - t2;
	return r;
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
	for (int a = -M; a < M; a++) {
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
	for (int a = -M; a < M; a++) {
		int aR = toMont(a);
		for (int b = -M; b < M; b++) {
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
	printf("a, b in [%d, %d]\n", -M, M);
	for (int a = -M; a < M; a++) {
		int aq = mull(a, q);
		for (int b = -M; b < M; b++) {
			int y = mont1(b, a, aq);
			range.update(y);
			int z = mont(a, b);
			if ((y - z) % p) {
				printf("ERR3 a=%d b=%d\n", a, b);
				printf("y = %d, z = %d\n", y, z);
				exit(1);
			}
		}
	}
	range.put("y");
	puts("ok");
}

int main()
{
//	printf("Q=%d\n", powMod(R, p-2,p));
//	printf("R=%d\n", (1-R*Q)/p);
	putParam();
	toMontTest();
	montTest();
	mont1Test();
}
