// gcc, clang : -fopenmp
// VC : /openmp
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>

static const uint64_t one = 1;
static const uint32_t N = 32;
static const uint64_t M = (one << N) - 1;

#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
typedef __attribute__((mode(TI))) unsigned int uint128_t;
#define MCL_DEFINED_UINT128_T
#endif

inline uint64_t mulUnit1(uint64_t *pH, uint64_t x, uint64_t y)
{
#ifdef MCL_DEFINED_UINT128_T
	uint128_t t = uint128_t(x) * y;
	*pH = uint64_t(t >> 64);
	return uint64_t(t);
#else
	return _umul128(x, y, pH);
#endif
}

uint32_t floor_ilog2(uint32_t x)
{
	uint32_t a = 0;
	while ((one << a) <= x) a++;
	return a - 1;
}

uint32_t ceil_ilog2(uint32_t x)
{
	uint32_t a = 0;
	while ((one << a) < x) a++;
	return a;
}

/*
M: integer >= 1.
d in [1, M]
take a. A := 2^a >= d. c := (A + d - 1) // d.
e = d c - A. 0 <= e <= d-1 < A.
(q0, r0) = divmod(M, d). M = q0 d + r0.
M0 := M - ((M+1)%d). Then M0%d = d-1.
Theorem
if e M0 < A, then (x c)//A = x//d. for x in [0, M].
Proof
(q, r) := divmod(x, d). x = d q + r.
Then x c = d q c + r c = (A + e) q + r c = A q + (q e + r c).
y := q e + r c.
If 0 <= y < A, then q = (x c) >> a.
So we prove that y < A if and only if e M0 < A.
y d = q e d + r c d = e q d + r (e + A) = e(d q + r) + r A = e x + r A.
Consider max(y d).
if r0 = d-1, then max(y d) (at x=M) = e M + (d-1) A = e M0 + (d-1) A.
if r0 < d-1, then max(y d) (at x=M0) = e M0 + (d-1) A because e < A.
Then max (y d) = e M0 + (d-1)A.
max(y) < A iff max(y d) < d A iff e M0 + (d-1) A < d A iff e M0 < A.
e M0 < A iff c/A < (1 + 1/M0)/d. This condition is the assumption of Thereom 1
in Integer division by constants: optimal bounds.
*/
struct MyAlgo {
	uint32_t d_;
	uint32_t a_;
	uint64_t A_;
	uint64_t c_;
	MyAlgo() : d_(0), a_(0), A_(0), c_(0) {}
	void put() const
	{
		printf("My d=%u(0x%08x) a=%u u=0x%" PRIx64 "\n", d_, d_, a_, c_);
	}
	bool init(uint32_t d)
	{
		assert(d <= M);
		uint32_t M0 = M - ((M+1)%d);
		// u > 0 => A >= d => a >= ilog2(d)
		for (uint32_t a = floor_ilog2(d); a < 64; a++) {
			uint64_t A = one << a;
			uint64_t c = (A + d - 1) / d;
			assert(c < (one << 33));
			if (c >= (one << 33)) continue; // same result if this line is comment out.
			uint64_t e = d * c - A;
			assert(e < A);
			if (e * M0 < A) {
				assert(e < c);
				d_ = d;
				a_ = a;
				A_ = A;
				c_ = c;
				return true;
			}
		}
		return false;
	}
	uint32_t divp(uint32_t x) const
	{
		if (c_ > 0xffffffff) {
#ifdef MCL_DEFINED_UINT128_T
			uint128_t v = (x * uint128_t(c_)) >> a_;
			return uint32_t(v);
#else
#if 1
			uint64_t v = x * (c_ & 0xffffffff);
			v >>= 32;
			v += x;
			v >>= a_-32;
			return uint32_t(v);
#else
			uint64_t H;
			uint64_t L = mulUnit1(&H, x, c);
			L >>= a;
			H <<= (64 - a);
			return uint32_t(H | L);
#endif
#endif
		} else {
			uint32_t v = uint32_t((x * c_) >> a_);
			return v;
		}
	}
};

// Division by Invariant Integers using Multiplication. Granlund, Montgomery
// https://dl.acm.org/doi/10.1145/178243.178249
struct GM {
	uint32_t d_;
	uint32_t l_;
	uint64_t mH_;
	uint32_t sh_;
	uint32_t a_; // same as MyAlgo
	void put() const
	{
		printf("GM d=%u(0x%08x) l=%u mH=0x%" PRIx64 " sh=0x%x a=%u\n", d_, d_, l_, mH_, sh_, a_);
	}
	bool init(uint32_t d)
	{
		uint32_t l = ceil_ilog2(d);
		uint32_t sh = l;
		uint64_t a = one << (N+l);
		uint64_t L = a / d;
//		uint64_t mH = (a + (one << l)) / d; // original Figure 6.2 of GM
		uint64_t mH = (a + (a / (M - (M % d)))) / d; // better parameter
		while ((L>>1) < (mH>>1) && sh > 0) {
			L >>= 1;
			mH >>= 1;
			sh--;
		}
		d_ = d;
		l_ = l;
		mH_ = mH;
		sh_ = sh;
		a_ = N + sh_;
		return true;
	}
	uint32_t divp(uint32_t x) const
	{
		if (mH_ > 0xffffffff) {
			uint32_t t1 = uint32_t(((mH_ - (M+1)) * x) >> N);
			uint32_t q = (t1 + ((x - t1) >> 1)) >> (sh_ - 1);
			return q;
		} else {
			uint32_t q = uint32_t((mH_ * x) >> (N + sh_));
			return q;
		}
	}
};

template<class Algo>
void check(const Algo& algo, uint32_t x)
{
	uint32_t q = algo.divp(x);
	uint32_t r = x - q * algo.d_;
	if (r >= algo.d_) {
		algo.put();
		printf("ERR x=%u\n", x);
		printf("ok=%u err=%u\n", q / algo.d_, q);
		exit(1);
	}
}

template<class Algo>
void checkAll(uint32_t d)
{
	Algo algo;
	if (!algo.init(d)) {
		printf("ERR algo.init(%u)\n", d);
		return;
	}
	algo.put();
	#pragma omp parallel for
	for (int64_t xx = 0; xx <= int64_t(M); xx++) {
		uint32_t x = uint32_t(xx);
		check(algo, x);
	}
}

template<class Algo>
void checkSomeP(const uint32_t *tbl, size_t tblN)
{
	for (size_t i = 0; i < tblN; i++) {
		uint32_t d = tbl[i];
		checkAll<Algo>(d);
	}
}

int main()
{
	{
		const uint32_t tbl[] = { 3, 5, 7, 10, 13, 0x7ffff, 68641, 6864137, /* 0xffffffff, */ };
		const size_t tblN = sizeof(tbl) / sizeof(tbl[0]);
		checkSomeP<MyAlgo>(tbl, tblN);
		checkSomeP<GM>(tbl, tblN);
	}
	// check MyAlgo
	{
		const uint32_t tbl[] = { 2, 4, 641, 65536, 0xb5062743, 0x7fffffff, 0x80000000, 0x80000001, 0xffffffff, };
		const size_t tblN = sizeof(tbl) / sizeof(tbl[0]);
		checkSomeP<MyAlgo>(tbl, tblN);
	}

#if 1
	{
		puts("find diff");
		#pragma omp parallel for
		for (int64_t d_ = 3; d_ <= 0xfffffffe; d_ += 2) {
			uint32_t d = uint32_t(d_);
			MyAlgo my;
			GM gm;
			bool b1 = my.init(d);
			bool b2 = gm.init(d);
			if (b1 && b2) {
				if (my.a_ != gm.a_) {
//					printf("d=%08x my=%u gm=%u\n", d, my.a_, gm.a_);
				}
				if (my.c_ <= M && gm.mH_ > M) {
					printf("d=0x%08x my.c=%" PRIx64 " a=%u gm.c=%" PRIx64 " a=%u\n", d, my.c_, my.a_, gm.mH_, gm.a_);
				}
			}
		}
		puts("ok");
	}
#endif


#if 1
	puts("all check");
	#pragma omp parallel for
	for (uint32_t d = 1; d <= 0xb0000000; d++) {
		MyAlgo algo;
		if (!algo.init(d)) {
			printf("ERR d=%u(0x%08x)\n", d, d);
//			checkAll(algo);
		}
	}
	puts("ok");
#endif
}
