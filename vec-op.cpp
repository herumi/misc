/*
clang++-15 -O2 vec-op.cpp -lgmp -lgmpxx -I ../cybozulib/include/ -mavx512f -mavx512ifma -Wall -Wextra && ./a.out
Xeon w9-3495X
uvadd  18.70 clk
uvsub  15.70 clk
uvmul 145.23 clk
*/
#include <stdint.h>
#include <stdio.h>
#include <gmpxx.h>
#include <iostream>
#include <cybozu/xorshift.hpp>
#include <cybozu/benchmark.hpp>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef uint64_t Unit;
typedef __m512i Vec;
typedef __mmask8 Vmask;

const size_t S = sizeof(Unit)*8-1; // 63
const size_t W = 52;
const size_t N = 8; // = ceil(384/52)
const size_t M = sizeof(Vec) / sizeof(Unit);
const uint64_t g_mask = (Unit(1)<<W) - 1;

const int C = 1000000;

static mpz_class g_mp;

static mpz_class g_mx, g_my, g_mr;

// split into 52 bits
static Unit g_p[N];

template<size_t N>
void toArray(Unit x[N], mpz_class mx)
{
	for (size_t i = 0; i < N; i++) {
		mpz_class a = mx & g_mask;
		x[i] = a.get_ui();
		mx >>= W;
	}
}

template<size_t N>
mpz_class fromArray(const Unit x[N])
{
	mpz_class mx = x[N-1];
	for (size_t i = 1; i < N; i++) {
		mx <<= W;
		mx += x[N-1-i];
	}
	return mx;
}

void put(const Unit *v, const char *msg = nullptr)
{
	if (msg) printf("%s ", msg);
	for (size_t i = 0; i < M; i++) {
		printf("%014lx ", v[M-1-i]);
	}
	printf("\n");
}

void put(const Vec& v, const char *msg = nullptr)
{
	put((const Unit*)&v, msg);
}

void put(const Vec *v, size_t n = N, const char *msg = nullptr)
{
	if (msg) printf("%s\n", msg);
	for (size_t i = 0; i < n; i++) {
		put(v[i]);
	}
}

void put(const Vmask& c, const char *msg = nullptr)
{
	if (msg) printf("%s ", msg);
	size_t n = sizeof(Vmask);
	const uint8_t *p = (const uint8_t*)&c;
	for (size_t i = 0; i < n; i++) {
		printf("%02x(%02x) ", p[n-1-i], uint8_t(~p[n-1-i]));
	}
	printf("\n");
}

Vec vzero()
{
	return _mm512_setzero_epi32();
}

static Vec vmask;
static Vec vrp;
static Vec vpN[N];

// set x[j] to i-th SIMD element of v[j]
void set(Vec v[N], size_t i, const Unit x[N])
{
	assert(i < M);
	Unit *p = (Unit *)v;
	for (size_t j = 0; j < N; j++) {
		p[j*M+i] = x[j];
	}
}

void get(Unit x[N], const Vec v[N], size_t i)
{
	assert(i < M);
	const Unit *p = (const Unit *)v;
	for (size_t j = 0; j < N; j++) {
		x[j] = p[j*M+i];
	}
}


void cvt(Vec yN[N], const Unit x[N*M])
{
	for (size_t i = 0; i < M; i++) {
		set(yN, i, x+i*N);
	}
}

void cvt(Unit y[N*M], const Vec xN[N])
{
	for (size_t i = 0; i < M; i++) {
		get(y+i*N, xN, i);
	}
}

// expand x to Vec
void expand(Vec& v, Unit x)
{
	Unit *p = (Unit *)&v;
	for (size_t i = 0; i < M; i++) {
		p[i] = x;
	}
}

void expandN(Vec v[N], const mpz_class& x)
{
	Unit a[N];
	toArray<N>(a, x);
	for (size_t i = 0; i < N; i++) {
		expand(v[i], a[i]);
	}
}

// low(c+a*b)
Vec vmulL(const Vec& a, const Vec& b, const Vec& c = vzero())
{
	return _mm512_madd52lo_epu64(c, a, b);
}

// high(c+a*b)
Vec vmulH(const Vec& a, const Vec& b, const Vec& c = vzero())
{
	return _mm512_madd52hi_epu64(c, a, b);
}

Vec vadd(const Vec& a, const Vec& b)
{
	return _mm512_add_epi64(a, b);
}

Vec vsub(const Vec& a, const Vec& b)
{
	return _mm512_sub_epi64(a, b);
}

Vec vshl(const Vec& a, int b)
{
	return _mm512_srli_epi64(a, b);
}

Vec vand(const Vec& a, const Vec& b)
{
	return _mm512_and_epi64(a, b);
}

// return [H:L][idx]
Vec vperm2tq(const Vec& L, const Vec& idx, const Vec& H)
{
	return _mm512_permutex2var_epi64(L, idx, H);
}

Vmask vcmpeq(const Vec& a, const Vec& b)
{
	return _mm512_cmpeq_epi64_mask(a, b);
}

Vmask vcmpneq(const Vec& a, const Vec& b)
{
	return _mm512_cmpneq_epi64_mask(a, b);
}

// return c ? a&b : d;
Vec vand(const Vmask& c, const Vec& a, const Vec& b, const Vec& d)
{
	return _mm512_mask_and_epi64(d, c, a, b);
}

Vec vselect(const Vmask& c, const Vec& a, const Vec& b)
{
	return vand(c, a, a, b);
}

void vrawAdd(Vec *z, const Vec *x, const Vec *y)
{
	Vec t = vadd(x[0], y[0]);
	Vec c = vshl(t, W);
	z[0] = vand(t, vmask);

	for (size_t i = 1; i < N; i++) {
		t = vadd(x[i], y[i]);
		t = vadd(t, c);
		if (i == N-1) {
			z[i] = t;
			return;
		}
		c = vshl(t, W);
		z[i] = vand(t, vmask);
	}
}

Vmask vrawSub(Vec *z, const Vec *x, const Vec *y)
{
	Vec t = vsub(x[0], y[0]);
	Vec c = vshl(t, S);
	z[0] = vand(t, vmask);
	for (size_t i = 1; i < N; i++) {
		t = vsub(x[i], y[i]);
		t = vsub(t, c);
		c = vshl(t, S);
		z[i] = vand(t, vmask);
	}
	return vcmpneq(c, vzero());
}

void uvselect(Vec *z, const Vmask& c, const Vec *a, const Vec *b)
{
	for (size_t i = 0; i < N; i++) {
		z[i] = vselect(c, a[i], b[i]);
	}
}

void uvadd(Vec *z, const Vec *x, const Vec *y)
{
	Vec sN[N], tN[N];
	vrawAdd(sN, x, y);
	Vmask c = vrawSub(tN, sN, vpN);
	uvselect(z, c, sN, tN);
}

void uvsub(Vec *z, const Vec *x, const Vec *y)
{
	Vec sN[N], tN[N];
	Vmask c = vrawSub(sN, x, y);
	vrawAdd(tN, sN, vpN);
	tN[N-1] = vand(tN[N-1], vmask);
	uvselect(z, c, tN, sN);
}

void vrawMulUnitOrg(Vec *z, const Vec *x, const Vec& y)
{
	Vec L[N], H[N];
	for (size_t i = 0; i < N; i++) {
		L[i] = vmulL(x[i], y);
		H[i] = vmulH(x[i], y);
	}
	z[0] = L[0];
	for (size_t i = 1; i < N; i++) {
		z[i] = vadd(L[i], H[i-1]);
	}
	z[N] = H[N-1];
}

Vec vrawMulUnitAddOrg(Vec *z, const Vec *x, const Vec& y)
{
	Vec L[N], H[N];
	for (size_t i = 0; i < N; i++) {
		L[i] = vmulL(x[i], y);
		H[i] = vmulH(x[i], y);
	}
	z[0] = vadd(z[0], L[0]);
	for (size_t i = 1; i < N; i++) {
		z[i] = vadd(z[i], vadd(L[i], H[i-1]));
	}
	return H[N-1];
}

void vrawMulUnit(Vec *z, const Vec *x, const Vec& y)
{
	Vec H;
	z[0] = vmulL(x[0], y);
	H = vmulH(x[0], y);
	for (size_t i = 1; i < N; i++) {
		z[i] = vmulL(x[i], y, H);
		H = vmulH(x[i], y);
	}
	z[N] = H;
}

Vec vrawMulUnitAdd(Vec *z, const Vec *x, const Vec& y)
{
	Vec H;
	z[0] = vmulL(x[0], y, z[0]);
	H = vmulH(x[0], y);
	for (size_t i = 1; i < N; i++) {
		z[i] = vadd(vmulL(x[i], y, H), z[i]);
		H = vmulH(x[i], y);
	}
	return H;
}

void uvmul(Vec *z, const Vec *x, const Vec *y)
{
	Vec t[N*2], q;
	vrawMulUnit(t, x, y[0]);
	q = vmulL(t[0], vrp);
	t[N] = vadd(t[N], vrawMulUnitAdd(t, vpN, q));
	for (size_t i = 1; i < N; i++) {
		t[N+i] = vrawMulUnitAdd(t+i, x, y[i]);
		t[i] = vadd(t[i], vshl(t[i-1], W));
		q = vmulL(t[i], vrp);
		t[N+i] = vadd(t[N+i], vrawMulUnitAdd(t+i, vpN, q));
	}
	for (size_t i = N; i < N*2; i++) {
		t[i] = vadd(t[i], vshl(t[i-1], W));
		t[i-1] = vand(t[i-1], vmask);
	}
	Vmask c = vrawSub(z, t+N, vpN);
	uvselect(z, c, t+N, z);
}

// out = c ? a : b
void select(Unit *out, bool c, const Unit *a, const Unit *b)
{
	const Unit *o = c ? a : b;
	for (size_t i = 0; i < N; i++) {
		out[i] = o[i];
	}
}

namespace mcl { namespace bint {
// ppLow = Unit(p)
inline Unit getMontgomeryCoeff(Unit pLow, size_t bitSize = sizeof(Unit) * 8)
{
	Unit pp = 0;
	Unit t = 0;
	Unit x = 1;
	for (size_t i = 0; i < bitSize; i++) {
		if ((t & 1) == 0) {
			t += pLow;
			pp += x;
		}
		t >>= 1;
		x <<= 1;
	}
	return pp;
}

} // mcl::bint

namespace gmp {

inline void set(mpz_class& z, uint64_t x)
{
	z = fromArray<1>(&x);
}
inline const Unit *getUnit(const mpz_class& x)
{
#ifdef MCL_USE_VINT
	return x.getUnit();
#else
	return reinterpret_cast<const Unit*>(x.get_mpz_t()->_mp_d);
#endif
}
inline Unit getUnit(const mpz_class& x, size_t i)
{
	return getUnit(x)[i];
}
inline size_t getUnitSize(const mpz_class& x)
{
#ifdef MCL_USE_VINT
	return x.getUnitSize();
#else
	return std::abs(x.get_mpz_t()->_mp_size);
#endif
}

} // mcl::gmp
} // mcl


class Montgomery {
	Unit v_[N];
public:
	mpz_class mp;
	mpz_class mR; // (1 << (N * 64)) % p
	mpz_class mR2; // (R * R) % p
	Unit rp; // rp * p = -1 mod M = 1 << 64
	const Unit *p;
	bool isFullBit;
	Montgomery() {}
	static Unit getLow(const mpz_class& x)
	{
		if (x == 0) return 0;
		return mcl::gmp::getUnit(x, 0) & g_mask;
	}
	void put() const
	{
		std::cout << std::hex;
		std::cout << "p=0x" << mp << std::endl;
		std::cout << "R=0x" << mR << std::endl;
		std::cout << "R2=0x" << mR2 << std::endl;
		std::cout << "rp=0x" << rp << std::endl;
	}
	void set(const mpz_class& _p)
	{
		mp = _p;
		mR = 1;
		mR = (mR << (W * N)) % mp;
		mR2 = (mR * mR) % mp;
		toArray<N>(v_, _p);
		::put(v_, "v_");
		rp = mcl::bint::getMontgomeryCoeff(v_[0], W);
		printf("rp=%zx\n", rp);
		p = v_;
		isFullBit = p[N-1] >> (W-1);
	}

	mpz_class toMont(const mpz_class& x) const
	{
		mpz_class y;
		mul(y, x, mR2);
		return y;
	}
	mpz_class fromMont(const mpz_class& x) const
	{
		mpz_class y;
		mul(y, x, 1);
		return y;
	}

	void mul(mpz_class& z, const mpz_class& x, const mpz_class& y) const
	{
		mod(z, x * y);
	}
	void mod(mpz_class& z, const mpz_class& xy) const
	{
		z = xy;
		for (size_t i = 0; i < N; i++) {
			Unit q = (getLow(z) * rp) & g_mask;
			mpz_class t = q;
			z += mp * t;
			z >>= W;
		}
		if (z >= mp) {
			z -= mp;
		}
	}
};

Montgomery g_mont;

void rawAdd(Unit *z, const Unit *x, const Unit *y)
{
	Unit c = 0;
	for (size_t i = 0; i < N; i++) {
		z[i] = x[i] + y[i] + c;
		if (i == N-1) break;
		c = z[i] >> W;
		z[i] &= g_mask;
	}
}

bool rawSub(Unit *z, const Unit *x, const Unit *y)
{
	Unit c = 0;
	for (size_t i = 0; i < N; i++) {
		z[i] = x[i] - y[i] - c;
		c = z[i] >> S;
		z[i] &= g_mask;
	}
	return c != 0;
}

void add(Unit *z, const Unit *x, const Unit *y)
{
	Unit s[N], t[N];
	rawAdd(s, x, y);
	bool c = rawSub(t, s, g_p);
	select(z, c, s, t);
}

void sub(Unit *z, const Unit *x, const Unit *y)
{
	Unit s[N], t[N];
	bool c = rawSub(s, x, y);
	rawAdd(t, s, g_p);
	t[N-1] &= g_mask;
	select(z, c, t, s);
}

uint64_t mulUnit1(uint64_t *pH, uint64_t x, uint64_t y)
{
#if !defined(_MSC_VER) || defined(__INTEL_COMPILER) || defined(__clang__)
	typedef __attribute__((mode(TI))) unsigned int uint128_t;
	uint128_t t = uint128_t(x) * y;
	*pH = uint64_t(t >> 64);
	return uint64_t(t);
#else
	return _umul128(x, y, pH);
#endif
}


// 52bit x 52bit = 104 bit
Unit mul52bit(Unit *pH, Unit x, Unit y)
{
	Unit L, H;
	L = mulUnit1(&H, x, y);
	*pH = (H << 12) | (L >> 52);
	return L & g_mask;
}

// z[N+1] = x[N] * y
void rawMulUnit(Unit *z, const Unit *x, Unit y)
{
	Unit L[N], H[N];
	for (size_t i = 0; i < N; i++) {
		L[i] = mul52bit(&H[i], x[i], y);
	}
	z[0] = L[0];
	for (size_t i = 1; i < N; i++) {
		z[i] = L[i] + H[i-1];
	}
	z[N] = H[N-1];
}

// [return:z[N+1]] += x[N] * y
Unit rawMulUnitAdd(Unit *z, const Unit *x, Unit y)
{
	Unit L[N], H[N];
	for (size_t i = 0; i < N; i++) {
		L[i] = mul52bit(&H[i], x[i], y);
	}
	z[0] += L[0];
	for (size_t i = 1; i < N; i++) {
		z[i] += L[i] + H[i-1];
	}
	return H[N-1];
}

// z[2N] = x[N] * y[N]
void rawMul(Unit *z, const Unit *x, const Unit *y)
{
	rawMulUnit(z, x, y[0]);
	for (size_t i = 1; i < N; i++) {
		z[N+i] = rawMulUnitAdd(z + i, x, y[i]);
	}
}

// z[N] = Montgomery mod(xy[2N])
void mod(Unit *z, const Unit *xy)
{
	Unit t[N*2], q, H;
	for (size_t i = 0; i < N*2; i++) {
		t[i] = xy[i];
	}
	for (size_t i = 0; i < N; i++) {
		q = mul52bit(&H, t[i], g_mont.rp);
		t[N+i] += rawMulUnitAdd(t + i, g_mont.p, q);
		t[i+1] += t[i] >> W;
		t[i] &= g_mask;
	}
	for (size_t i = N; i < N*2-1; i++) {
		t[i+1] += t[i] >> W;
		t[i] &= g_mask;
	}
	bool c = rawSub(z, t + N, g_mont.p);
	select(z, c, t + N, z);
}

// z[N] = Montgomery mul(x[N], y[2N])
void mul(Unit *z, const Unit *x, const Unit *y)
{
	Unit t[N*2], q, H;
	rawMulUnit(t, x, y[0]);
	q = mul52bit(&H, t[0], g_mont.rp);
	t[N] += rawMulUnitAdd(t, g_mont.p, q);
	for (size_t i = 1; i < N; i++) {
		t[N+i] = rawMulUnitAdd(t+i, x, y[i]);
		t[i] += t[i-1] >> W;
		q = mul52bit(&H, t[i], g_mont.rp);
		t[N+i] += rawMulUnitAdd(t+i, g_mont.p, q);
	}
	for (size_t i = N; i < N*2; i++) {
		t[i] += t[i-1] >> W;
		t[i-1] &= g_mask;
	}
	bool c = rawSub(z, t+N, g_mont.p);
	select(z, c, t+N, z);
}

template<class RG>
mpz_class mpz_rand(RG& rg)
{
	Unit x[N];
	for (size_t i = 0; i < N; i++) {
		x[i] = rg.get64();
	}
	mpz_class mx = fromArray<N>(x);
	return mx % g_mp;
}

mpz_class madd(const mpz_class& x, const mpz_class& y)
{
	mpz_class z = x + y;
	if (z >= g_mp) z -= g_mp;
	return z;
}

mpz_class msub(const mpz_class& x, const mpz_class& y)
{
	mpz_class z = x - y;
	if (z < 0) z += g_mp;
	return z;
}

void putAll(const mpz_class& x, const mpz_class& y, const mpz_class& z, const mpz_class& w)
{
	std::cout << "x=" << x << std::endl;
	std::cout << "y=" << y << std::endl;
	std::cout << "z=" << z << std::endl;
	std::cout << "w=" << w << std::endl;
}

template<class F>
void mul12(F& x)
{
	F t;
	F::add(t, x, x);
	F::add(x, t, x); // 3x
	F::add(x, x, x);
	F::add(x, x, x);
}
// 14M+19A
template<class E>
void addCTProj(E& R, const E& P, const E& Q)
{
	typedef typename E::Fp F;
	assert(E::a_ == 0 && E::b_ == 4);
	F t0, t1, t2, t3, t4, x3, y3;
	F::mul(t0, P.x, Q.x);
	F::mul(t1, P.y, Q.y);
	F::mul(t2, P.z, Q.z);
	F::add(t3, P.x, P.y);
	F::add(t4, Q.x, Q.y);
	F::mul(t3, t3, t4);
	F::add(t4, t0, t1);
	F::sub(t3, t3, t4);
	F::add(t4, P.y, P.z);
	F::add(x3, Q.y, Q.z);
	F::mul(t4, t4, x3);
	F::add(x3, t1, t2);
	F::sub(t4, t4, x3);
	F::add(x3, P.x, P.z);
	F::add(y3, Q.x, Q.z);
	F::mul(x3, x3, y3);
	F::add(y3, t0, t2);
	F::sub(y3, x3, y3);
	F::add(x3, t0, t0);
	F::add(t0, t0, x3);
#if 1
	mul12(t2);
#else
	F::mul(t2, t2, E::b3_);
#endif
	F::add(R.z, t1, t2);
	F::sub(t1, t1, t2);
#if 1
	mul12(y3);
#else
	F::mul(y3, y3, E::b3_);
#endif
	F::mul(x3, y3, t4);
	F::mul(t2, t3, t1);
	F::sub(R.x, t2, x3);
	F::mul(y3, y3, t0);
	F::mul(t1, t1, R.z);
	F::add(R.y, y3, t1);
	F::mul(t0, t0, t3);
	F::mul(R.z, R.z, t4);
	F::add(R.z, R.z, t0);
}
// 7M+2S+9A
template<class E>
void dblCTProj(E& R, const E& P)
{
	typedef typename E::Fp F;
	assert(E::a_ == 0 && E::b_ == 4);
	F t0, t1, t2, x3, y3;
	F::sqr(t0, P.y);
	F::mul(t1, P.y, P.z);
	F::sqr(t2, P.z);
	F::add(R.z, t0, t0);
	F::add(R.z, R.z, R.z);
	F::add(R.z, R.z, R.z);
#if 1
	mul12(t2);
#else
	F::mul(t2, t2, E::b3_);
#endif
	F::mul(x3, t2, P.z);
	F::add(y3, t0, t2);
	F::mul(R.z, R.z, t1);
	F::add(t1, t2, t2);
	F::add(t2, t2, t1);
	F::mul(t1, P.x, P.y);
	F::sub(t0, t0, t2);
	F::mul(R.y, y3, t0);
	F::add(R.y, R.y, x3);
	F::mul(R.x, t0, t1);
	F::add(R.x, R.x, R.x);
}

struct Fp {
	mpz_class v;
	Fp(int _v = 0) : v(_v) {}
	static void add(Fp& z, const Fp& x, const Fp& y)
	{
		z.v = madd(x.v, y.v);
	}
	static void sub(Fp& z, const Fp& x, const Fp& y)
	{
		z.v = msub(x.v, y.v);
	}
	static void mul(Fp& z, const Fp& x, const Fp& y)
	{
		g_mont.mul(z.v, x.v, y.v);
	}
	static void sqr(Fp& z, const Fp& x)
	{
		mul(z, x, x);
	}
	void set(const mpz_class& x)
	{
		v = g_mont.toMont(x);
	}
	mpz_class get() const
	{
		return g_mont.fromMont(v);
	}
};

struct Ec {
	typedef Fp Fp;
	static const int a_ = 0;
	static const int b_ = 4;
	static Fp b3_;
	static Ec zero_;
	Fp x, y, z;
	static void add(Ec& z, const Ec& x, const Ec& y)
	{
		addCTProj(z, x, y);
	}
	static void dbl(Ec& z, const Ec& x)
	{
		dblCTProj(z, x);
	}
	static void init(Montgomery& mont)
	{
		const int b = 4;
		b3_.v = mont.toMont(b * 3);
	}
	static const Ec& zero()
	{
		return zero_;
	}
	void clear()
	{
		*this = zero();
	}
	void set(const mpz_class& _x, const mpz_class& _y, const mpz_class& _z)
	{
		x.set(_x);
		y.set(_y);
		z.set(_z);
	}
	static void mul(Ec& Q, const Ec& P, Unit y)
	{
		const int w = 4;
		const int tblN = 1<<w;
		const int mask = tblN-1;
		Ec tbl[tblN];
		tbl[0].clear();
		tbl[1] = P;
		for (size_t i = 2; i < tblN; i++) {
			add(tbl[i], tbl[i-1], P);
		}
		const size_t bitLen = sizeof(Unit)*8;
		Q = tbl[(y >> (bitLen - w)) & mask];
		for (int j = (int)bitLen - w*2; j >= 0; j -= w) {
			for (size_t i = 0; i < w; i++) {
				Ec::dbl(Q, Q);
			}
			size_t idx = (y >> j) & mask;
			Ec::add(Q, Q, tbl[idx]);
		}
	}
};

Fp Ec::b3_;
Ec Ec::zero_;

struct FpM {
	Vec v[N];
	static void add(FpM& z, const FpM& x, const FpM& y)
	{
		uvadd(z.v, x.v, y.v);
	}
	static void sub(FpM& z, const FpM& x, const FpM& y)
	{
		uvsub(z.v, x.v, y.v);
	}
	static void mul(FpM& z, const FpM& x, const FpM& y)
	{
		uvmul(z.v, x.v, y.v);
	}
	static void sqr(FpM& z, const FpM& x)
	{
		uvmul(z.v, x.v, x.v);
	}
	void set(const mpz_class& x, size_t i)
	{
		Unit xv[N];
		toArray<N>(xv, x);
		::set(v, i, xv);
	}
	mpz_class get(size_t i) const
	{
		Unit x[N];
		::get(x, v, i);
		return fromArray<N>(x);
	}
};

struct EcM {
	typedef FpM Fp;
	static const int a_ = 0;
	static const int b_ = 4;
	static FpM b3_;
	static EcM zero_;
	FpM x, y, z;
	static void add(EcM& z, const EcM& x, const EcM& y)
	{
		addCTProj(z, x, y);
	}
	static void dbl(EcM& z, const EcM& x)
	{
		dblCTProj(z, x);
	}
	static void init(Montgomery& mont)
	{
		const int b = 4;
		mpz_class b3 = mont.toMont(b * 3);
		expandN(b3_.v, b3);
	}
	static const EcM& zero()
	{
		return zero_;
	}
	void clear()
	{
		*this = zero();
	}
	void set(const Ec& v, size_t i)
	{
		x.set(v.x.v, i);
		y.set(v.y.v, i);
		z.set(v.z.v, i);
	}
	void set(const Ec& v)
	{
		for (size_t i = 0; i < M; i++) {
			set(v, i);
		}
	}
	void set(const Ec v[M])
	{
		for (size_t i = 0; i < M; i++) {
			set(v[i], i);
		}
	}
};

FpM EcM::b3_;
EcM EcM::zero_;

void init(Montgomery& mont)
{
	const char *pStr = "1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab";
	g_mp.set_str(pStr, 16);
	mont.set(g_mp);
	toArray<N>(g_p, g_mp);
	expand(vmask, g_mask);
	expandN(vpN, g_mp);
	expand(vrp, mont.rp);
	Ec::init(mont);
	EcM::init(mont);
	Ec::zero_.set(0, 1, 0);
	EcM::zero_.set(Ec::zero());

	g_mx.set_str("17f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb", 16);
	g_my.set_str("08b3f481e3aaa0f1a09e30ed741d8ae4fcf5e095d5d00af600db18cb2c04b3edd03cc744a2888ae40caa232946c5e7e1", 16);
	g_mr.set_str("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001", 16);
	printf("(mx, my) is on %d\n", (g_mx * g_mx * g_mx + 4 - g_my * g_my) % g_mp == 0);
}

template<typename T>
void assertEq(const T& a, const T& b, const char *msg = nullptr)
{
	if (a != b) {
		if (msg) std::cout << msg << std::endl;
		std::cout << "a=" << a << std::endl;
		std::cout << "b=" << b << std::endl;
		exit(1);
	}
}

void test(const mpz_class& mx, const mpz_class& my)
{
	mpz_class mz, mw;
	Unit x[N], y[N], z[N];
	toArray<N>(x, mx);
	toArray<N>(y, my);

	mz = madd(mx, my);
	add(z, x, y);
	mw = fromArray<N>(z);
	if (mz != mw) {
		puts("err add\n");
		putAll(mx, my, mz, mw);
	}

	mz = msub(mx, my);
	sub(z, x, y);
	mw = fromArray<N>(z);
	if (mz != mw) {
		puts("err sub\n");
		putAll(mx, my, mz, mw);
	}
}

void vtest(const mpz_class& _mx, const mpz_class& _my)
{
	mpz_class mz, mw;
	mpz_class mx[M], my[M];
	alignas(64) Unit _x[N*M], _y[N*M], _z[N*M];
	Vec xN[N*M], yN[N*M], zN[N*M];
	for (size_t i = 0; i < M; i++) {
		mx[i] = (_mx + i * 123) % g_mp;
		my[i] = (_my + i * 245) % g_mp;
		toArray<N>(_x + i*N, mx[i]);
		toArray<N>(_y + i*N, my[i]);
	}

	cvt(xN, _x);
	cvt(yN, _y);

	// add
	for (size_t i = 0; i < M; i++) {
		uvadd(zN+i*N, xN+i*N, yN+i*N);
	}
	cvt(_z, zN);

	for (size_t i = 0; i < M; i++) {
		mz = madd(mx[i], my[i]);
		mw = fromArray<N>(_z + i*N);
		if (mz != mw) {
			printf("uvadd err %zd\n", i);
			putAll(mx[i], my[i], mz, mw);
		}
	}

	// sub
	for (size_t i = 0; i < M; i++) {
		uvsub(zN+i*N, xN+i*N, yN+i*N);
	}
	cvt(_z, zN);

	for (size_t i = 0; i < M; i++) {
		mz = msub(mx[i], my[i]);
		mw = fromArray<N>(_z + i*N);
		if (mz != mw) {
			printf("uvsub err %zd\n", i);
			putAll(mx[i], my[i], mz, mw);
		}
	}

#if 1
	// mulUnit
	{
		Vec z1[M+1], z2[M+1];
		memset(z1, 0, sizeof(z1));
		memset(z2, 0, sizeof(z2));
		vrawMulUnitOrg(z1, xN, yN[0]);
		vrawMulUnit(z2, xN, yN[0]);
		if (memcmp(z1, z2, sizeof(z1)) != 0) {
			for (size_t i = 0; i < N; i++) {
				printf("%zd ", i);
				put(xN[i], "x");
			}
			put(yN[0], "y");
			for (size_t i = 0; i < M+1; i++) {
				printf("i=%zd mulUnit %c\n", i, memcmp(&z1[i], &z2[i], sizeof(z1[0])) == 0 ? 'o' : 'x');
				put(z1[i], "z1");
				put(z2[i], "z2");
			}
		}
	}
#endif

	// mul
	for (size_t i = 0; i < M; i++) {
		uvmul(zN+i*N, xN+i*N, yN+i*N);
	}
	cvt(_z, zN);
	for (size_t i = 0; i < M; i++) {
		g_mont.mul(mz, mx[i], my[i]);
		mw = fromArray<N>(_z + i*N);
		if (mz != mw) {
			printf("uvmul err %zd\n", i);
			putAll(mx[i], my[i], mz, mw);
		}
	}
return;
	CYBOZU_BENCH_C("uvadd", C, uvadd, xN, xN, yN);
	CYBOZU_BENCH_C("uvsub", C, uvsub, xN, xN, yN);
	CYBOZU_BENCH_C("uvmul", C, uvmul, xN, xN, yN);
}

void testMont(const mpz_class& mx, const mpz_class& my)
{
	mpz_class ax, ay, axy, xy, mxy;
	ax = g_mont.toMont(mx);
	ay = g_mont.toMont(my);
	g_mont.mul(axy, ax, ay);
	xy = g_mont.fromMont(axy);
	mxy = (mx * my) % g_mp;
	if (xy != mxy) {
		puts("err g_mont");
		putAll(mx, my, mxy, xy);
	}
	Unit x[N], y[N], t[N*2], z[N];
	toArray<N>(x, ax);
	toArray<N>(y, ay);
	rawMul(t, x, y);
	mod(z, t);
	mpz_class w = fromArray<N>(z);
	if (w != axy) {
		puts("err mont2");
		putAll(mx, my, axy, w);
	}
	memset(z, 0, sizeof(z));
	mul(z, x, y);
	w = fromArray<N>(z);
	if (w != axy) {
		puts("err mont3");
		putAll(mx, my, axy, w);
	}
}

void cmpEc(const EcM& P, const Ec Q[M], const char *msg = nullptr)
{
	if (msg) printf("%s\n", msg);
	for (size_t i = 0; i < M; i++) {
		assertEq(P.x.get(i), Q[i].x.v, "x");
		assertEq(P.y.get(i), Q[i].y.v, "y");
		assertEq(P.z.get(i), Q[i].z.v, "z");
	}
}

void ecTest()
{
	puts("ecTest");
	Ec P1[M], Q1[M], R1[M];
	EcM P2, Q2, R2;
	P1[0].x.set(g_mx);
	P1[0].y.set(g_my);
	P1[0].z.set(1);
	for (size_t i = 1; i < M; i++) {
		Ec::dbl(P1[i], P1[i-1]);
	}
	P2.set(P1);
	cmpEc(P2, P1, "P");

	for (size_t i = 0; i < M; i++) {
		Ec::add(Q1[i], P1[i], P1[M-1]);
	}
	Q2.set(Q1);
	cmpEc(Q2, Q1, "Q");

	for (size_t i = 0; i < M; i++) {
		Ec::add(R1[i], P1[i], Q1[i]);
	}
	EcM::add(R2, P2, Q2);
	cmpEc(R2, R1, "R");


	for (size_t i = 0; i < M; i++) {
		Ec::dbl(R1[i], R1[i]);
	}
	EcM::dbl(R2, R2);
	cmpEc(R2, R1, "R2");
	CYBOZU_BENCH_C("EcM::add", C, EcM::add, R2, R2, Q2);
	CYBOZU_BENCH_C("EcM::dbl", C, EcM::dbl, R2, R2);
	CYBOZU_BENCH_C("FpM::add", C, FpM::add, R2.x, R2.x, Q2.x);
	CYBOZU_BENCH_C("FpM::sub", C, FpM::sub, R2.x, R2.x, Q2.x);
	CYBOZU_BENCH_C("FpM::mul", C, FpM::mul, R2.x, R2.x, Q2.x);
}

void mulTest()
{
	puts("mulTest");
	Ec P1[M], Q1[M];
	EcM P2, Q2;
	P1[0].x.set(g_mx);
	P1[0].y.set(g_my);
	P1[0].z.set(1);
	for (size_t i = 1; i < M; i++) {
		Ec::dbl(P1[i], P1[i-1]);
	}
	P2.set(P1);
	Ec::mul(Q1[0], P1[0], 123456789);
	std::cout << "X=" << Q1[0].x.get() << std::endl;
	std::cout << "Y=" << Q1[0].y.get() << std::endl;
	std::cout << "Z=" << Q1[0].z.get() << std::endl;
}

void testAll(const mpz_class& mx, const mpz_class& my)
{
	test(mx, my);
	testMont(mx, my);
	vtest(mx, my);
}

void miscTest()
{
	Unit v[] = {
		0x1234000, 0x1234001, 0x1234002, 0x1234003, 0x1234004, 0x1234005, 0x1234006, 0x1234007,
		0x1234008, 0x1234009, 0x123400a, 0x123400b, 0x123400c, 0x123400d, 0x123400e, 0x123400f,
	};
	Unit idx1[] = {
		0, 2, 4, 6, 8, 10, 12, 14
	};
	Unit idx2[] = {
		15, 13, 11, 9, 7, 5, 3, 1
	};
	Vec a = *(const Vec*)&v[0];
	Vec b = *(const Vec*)&v[8];
	Vec idx = *(const Vec*)idx1;
	Vec x = vperm2tq(a, idx, b);
	put(x, "idx1");
	idx = *(const Vec*)idx2;
	x = vperm2tq(a, idx, b);
	put(x, "idx2");
}

int main()
{
	init(g_mont);
	g_mont.put();

	const mpz_class tbl[] = {
		0xaabbccdd, 0x11223344, 0, 1, 2, g_mask-1, g_mask, g_mask+1, g_mp-1, g_mp>>2, 0x12345, g_mp-0x1111,
	};
	std::cout << std::hex;
	for (const auto& mx : tbl) {
		for (const auto& my : tbl) {
			testAll(mx, my);
		}
	}

	cybozu::XorShift rg;
	for (int i = 0; i < 100; i++) {
		mpz_class mx = mpz_rand(rg);
		mpz_class my = mpz_rand(rg);
		testAll(mx, my);
	}
	miscTest();
	mulTest();
	ecTest();
	puts("ok");
}
