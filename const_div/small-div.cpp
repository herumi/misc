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

uint32_t floor_ilog2(uint32_t p)
{
	uint32_t a = 0;
	while ((one << a) <= p) a++;
	return a - 1;
}

uint32_t ceil_ilog2(uint32_t p)
{
	uint32_t a = 0;
	while ((one << a) < p) a++;
	return a;
}

/*
M = 2**32 - 1
p in [1, M]
2^a = p u - e, 0 <= e <= p-1
x in [0, M]. (q, r) := divmod(x, p). x = p q + r.
Then x u = p q u + r0 u = (2^a + e) q + r u = 2^a q + (e q + r u)
y := q e + r u.
If 0 <= y < 2^a, then q = (x u) >> a.
To satisfy the condition, compute max(y).
q0 := M//p. r0 := M%p.
1-1) q = q0 and r = r0 then y = q0 e + r0 u
1-2) q = q0-1 and r0 = p-1 then y = (q0-1)e + (p-1)u
Then max(q0 e + r0 u, (q0-1)e + (p-1)u) < 2^a.
*/
struct MyAlgo {
	uint32_t p_;
	uint32_t a_;
	uint64_t A_;
	uint64_t u_;
	MyAlgo() : p_(0), a_(0), A_(0), u_(0) {}
	void put() const
	{
		printf("My p=%u(0x%08x) a=%u u=0x%" PRIx64 "\n", p_, p_, a_, u_);
	}
	bool init(uint32_t p)
	{
		assert(p <= M);
		uint32_t q0 = M / p;
		uint32_t r0 = M % p;
		// u > 0 => A >= p => a >= ilog2(p)
		for (uint32_t a = floor_ilog2(p); a < 64; a++) {
			uint64_t A = one << a;
			uint64_t u = (A + p - 1) / p;
			assert(u < (one << 33));
			if (u >= (one << 33)) continue; // same result if this line is comment out.
			uint64_t e = p * u - A;
//			if ((q0-1) * e + (p-1) * u < A && q0 * e + r0 * u < A) { // better
//			if (q0 * e < u && (q0+1) * e < (p-r0) * u) { // equivalent
			if (e * (M-((M+1)%p)) < A) { // Integer division by constants: optimal bounds. Theorem 1
				p_ = p;
				a_ = a;
				A_ = A;
				u_ = u;
				return true;
			}
		}
		return false;
	}
	uint32_t divp(uint32_t x) const
	{
		if (u_ > 0xffffffff) {
#ifdef MCL_DEFINED_UINT128_T
			uint128_t v = (x * uint128_t(u_)) >> a_;
			return uint32_t(v);
#else
#if 1
			uint64_t v = x * (u_ & 0xffffffff);
			v >>= 32;
			v += x;
			v >>= a_-32;
			return uint32_t(v);
#else
			uint64_t H;
			uint64_t L = mulUnit1(&H, x, u);
			L >>= a;
			H <<= (64 - a);
			return uint32_t(H | L);
#endif
#endif
		} else {
			uint32_t v = uint32_t((x * u_) >> a_);
			return v;
		}
	}
};

// Division by Invariant Integers using Multiplication. Granlund, Montgomery
// https://dl.acm.org/doi/10.1145/178243.178249
struct GM {
	uint32_t p_;
	uint32_t l_;
	uint64_t mH_;
	uint32_t sh_;
	uint32_t a_; // same as MyAlgo
	void put() const
	{
		printf("GM p=%u(0x%08x) l=%u mH=0x%" PRIx64 " sh=0x%x a=%u\n", p_, p_, l_, mH_, sh_, a_);
	}
	bool init(uint32_t p)
	{
		uint32_t l = ceil_ilog2(p);
		uint32_t sh = l;
		uint64_t a = one << (N+l);
		uint64_t L = a / p;
//		uint64_t mH = (a + (one << l)) / p; // original Figure 6.2 of GM
		uint64_t mH = (a + (a / (M - (M % p)))) / p; // better parameter
		while ((L>>1) < (mH>>1) && sh > 0) {
			L >>= 1;
			mH >>= 1;
			sh--;
		}
		p_ = p;
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
	uint32_t r = x - q * algo.p_;
	if (r >= algo.p_) {
		algo.put();
		printf("ERR x=%u\n", x);
		printf("ok=%u err=%u\n", q / algo.p_, q);
		exit(1);
	}
}

template<class Algo>
void checkAll(uint32_t p)
{
	Algo algo;
	if (!algo.init(p)) {
		printf("ERR algo.init(%u)\n", p);
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
		uint32_t p = tbl[i];
		checkAll<Algo>(p);
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
		for (int64_t p_ = 3; p_ <= 0xfffffffe; p_ += 2) {
			uint32_t p = uint32_t(p_);
			MyAlgo my;
			GM gm;
			bool b1 = my.init(p);
			bool b2 = gm.init(p);
			if (b1 && b2) {
				if (my.a_ != gm.a_) {
//					printf("p=%08x my=%u gm=%u\n", p, my.a_, gm.a_);
				}
				if (my.u_ <= M && gm.mH_ > M) {
					printf("p=0x%08x my.u=%" PRIx64 " a=%u gm.u=%" PRIx64 " a=%u\n", p, my.u_, my.a_, gm.mH_, gm.a_);
				}
			}
		}
		puts("ok");
	}
#endif


#if 0
	puts("all check");
	#pragma omp parallel for
	for (uint32_t p = 0x7fffffff; p <= 0xfffffffe; p += 2) {
		MyAlgo algo;
		if (algo.init(p)) {
			checkAll(algo);
		}
	}
	puts("ok");
#endif
}
