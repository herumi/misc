#include <mcl/bls12_381.hpp>
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <stdio.h>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>

using namespace mcl;
using namespace mcl::bn;

extern "C" int invModPre_org(int *py, int x, int m);
extern "C" int invModPre4(uint64_t *py, const uint64_t *px, const uint64_t *pm);
extern "C" int invModPre6(uint64_t *py, const uint64_t *px, const uint64_t *pm);
typedef int (*invModPreType)(uint64_t *, const uint64_t *, const uint64_t *);
#define AAA

// assert m is not full bit
// y s.t. x y = 1 mod m
int invModPre(Vint& y, const Vint& x, const Vint& m)
{
	Vint u = m;
	Vint v = x;
	Vint r = 0;
	Vint s = 1;
	int k = 0;
	while (!v.isZero()) {
		if (u.isEven()) {
			u >>= 1;
			s <<= 1;
		} else if (v.isEven()) {
			v >>= 1;
			r <<= 1;
		} else {
			if (u > v) {
				u -= v;
				u >>= 1;
				r += s;
				s <<= 1;
			} else {
				v -= u;
				v >>= 1;
				s += r;
				r <<= 1;
			}
		}
		k++;
	}
	if (r > m) {
		r -= m;
	}
	y = m - r;
	return k;
}

void invMod(Vint& y, const Vint& x, const Vint& m)
{
	int k = invModPre(y, x, m);
#ifdef AAA
	{
		uint64_t ys[4];
		int k2 = invModPre4(ys, x.getUnit(), m.getUnit());
		CYBOZU_TEST_EQUAL(k, k2);
		CYBOZU_TEST_EQUAL_ARRAY(ys, y.getUnit(), 4);
	}
#endif
	Vint h = (m + 1) >> 1;
	while (k > 0) {
		bool odd = y.isOdd();
		y >>= 1;
		if (odd) {
			y += h;
		}
		k--;
	}
}

template<class F, size_t N>
void bench(invModPreType f)
{
	const int C = 1000;
	F x;
	Unit y1[N], y2[N];
	cybozu::XorShift rg;
	const fp::Op& op = F::getOp();
	const Unit *p = op.p;
	for (int i = 0; i < 1000; i++) {
		x.setRand(rg);
		int k1 = op.fp_preInv(y1, x.getUnit());
		int k2 = f(y2, x.getUnit(), p);
		CYBOZU_TEST_EQUAL(k1, k2);
		CYBOZU_TEST_EQUAL_ARRAY(y1, y2, op.N);
	}
	CYBOZU_BENCH_C("asm", C, y1[0]++;op.fp_preInv, y1, y1);
	CYBOZU_BENCH_C("llv", C, y2[0]++;f, y2, y2, p);
	CYBOZU_TEST_EQUAL_ARRAY(y1, y2, op.N);
}

int main()
{
	initPairing(mcl::BLS12_381);
	Vint x, y, m, r;
	int ux, uy, um = 997;
	for (int i = 1; i < um; i++) {
		ux = i;
		int k1 = invModPre(y, ux, um);
		int k2 = invModPre_org(&uy, ux, um);
		if (y != uy) {
			printf("err i=%d y=%s yu=%d\n", i, y.getStr().c_str(), uy);
		}
		if (k1 != k2) {
			printf("err i=%d k1=%d, k2=%d\n", i, k1, k2);
		}
	}
	m = um;
	for (int i = 1; i < m; i++) {
		x = i;
		invMod(y, x, m);
		r = (x * y) % m;
		if (r != 1) {
			printf("ERR x=%s y=%s : %s\n", x.getStr().c_str(), y.getStr().c_str(), r.getStr().c_str());
		}
	}
	puts("ok");
	bench<Fp, 6>(invModPre6);
	bench<Fr, 4>(invModPre4);
}
