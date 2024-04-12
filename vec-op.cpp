/*
make CFLAGS_USER=-DMCL_USE_GMP=1 lib/libmcl.a
clang++-15 -O2 vec-op.cpp -lgmp -lgmpxx -I ../cybozulib/include/ -mavx512f -mavx512ifma -Wall -Wextra -I ../mcl/include/ ../mcl/lib/libmcl.a && ./a.out
Xeon w9-3495X
uvadd  18.70 clk
uvsub  15.70 clk
uvmul 145.23 clk
*/
#define MCL_USE_GMP
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <cybozu/xorshift.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/option.hpp>
#include <mcl/bls12_381.hpp>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef mcl::Unit Unit;
typedef __m512i Vec;
typedef __mmask8 Vmask;

const size_t S = sizeof(Unit)*8-1; // 63
const size_t W = 52;
const size_t N = 8; // = ceil(384/52)
const size_t M = sizeof(Vec) / sizeof(Unit);
const uint64_t g_mask = (Unit(1)<<W) - 1;

const int C = 1000000;

static mpz_class g_mp, g_mr;

static mpz_class g_mx, g_my;

static mpz_class g_rw;

// split into 52 bits
static Unit g_p[N];

static Unit g_mpM2[6]; // x^(-1) = x^(p-2) mod p

static const Unit g_L[2] = { 0x00000000ffffffff, 0xac45a4010001a402 };

static Vec vmask;
static Vec vrp;
static Vec vpN[N];
static Vec g_vmpM2[6]; // NOT 52-bit but 64-bit
static Vec g_vmask4;
static Vec g_offset;
static Vec g_vi192;

std::ostream& operator<<(std::ostream& os, const Vec& v)
{
	const Unit *p = (const Unit *)&v;
	char buf[64];
	for (size_t i = 0; i < M; i++) {
		snprintf(buf, sizeof(buf), "%016lx ", p[M-1-i]);
		os << buf;
	}
	return os;
}

Unit getMask(int w)
{
	if (w == 64) return Unit(-1);
	return (Unit(1) << w) - 1;
}

template<size_t N, int w = W>
void toArray(Unit x[N], mpz_class mx)
{
	const Unit mask = getMask(w);
	for (size_t i = 0; i < N; i++) {
		mpz_class a = mx & mask;
		x[i] = a.get_ui();
		mx >>= w;
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

void put(const Unit *v, const char *msg = nullptr, size_t m = M)
{
	if (msg) printf("%s ", msg);
	for (size_t i = 0; i < m; i++) {
		printf("%014lx ", v[m-1-i]);
	}
	printf("\n");
}

void put(const Vec& v, const char *msg = nullptr)
{
	put((const Unit*)&v, msg);
}

void put(const Vec *v, const char *msg = nullptr, size_t n = N)
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

Vec vpsrlq(const Vec& a, int b)
{
	return _mm512_srli_epi64(a, b);
}

Vec vpsllq(const Vec& a, int b)
{
	return _mm512_slli_epi64(a, b);
}


Vec vand(const Vec& a, const Vec& b)
{
	return _mm512_and_epi64(a, b);
}

Vec vor(const Vec& a, const Vec& b)
{
	return _mm512_or_epi64(a, b);
}

//template<int scale=8>
Vec vpgatherqq(const Vec& idx, const void *base)
{
#if 0
	const Unit *p = (const Unit *)&idx;
	const Unit *src = (const Unit *)base;
	Vec v;
	Unit *q = (Unit *)&v;
	for (size_t i = 0; i < M; i++) {
		q[i] = src[idx[i]];
	}
	return v;
#else
	const int scale = 8;
	return _mm512_i64gather_epi64(idx, base, scale);
#endif
}

void vpscatterqq(void *base, const Vec& idx, const Vec& v)
{
	const int scale = 8;
	_mm512_i64scatter_epi64(base, idx, v, scale);
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

Vec vpbroadcastq(int64_t a)
{
	return _mm512_set1_epi64(a);
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
	Vec c = vpsrlq(t, W);
	z[0] = vand(t, vmask);

	for (size_t i = 1; i < N; i++) {
		t = vadd(x[i], y[i]);
		t = vadd(t, c);
		if (i == N-1) {
			z[i] = t;
			return;
		}
		c = vpsrlq(t, W);
		z[i] = vand(t, vmask);
	}
}

Vmask vrawSub(Vec *z, const Vec *x, const Vec *y)
{
	Vec t = vsub(x[0], y[0]);
	Vec c = vpsrlq(t, S);
	z[0] = vand(t, vmask);
	for (size_t i = 1; i < N; i++) {
		t = vsub(x[i], y[i]);
		t = vsub(t, c);
		c = vpsrlq(t, S);
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
		t[i] = vadd(t[i], vpsrlq(t[i-1], W));
		q = vmulL(t[i], vrp);
		t[N+i] = vadd(t[N+i], vrawMulUnitAdd(t+i, vpN, q));
	}
	for (size_t i = N; i < N*2; i++) {
		t[i] = vadd(t[i], vpsrlq(t[i-1], W));
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

Vec getUnitAt(const Vec *x, size_t xN, size_t bitPos)
{
	const size_t bitSize = 64;
	const size_t q = bitPos / bitSize;
	const size_t r = bitPos % bitSize;
	if (r == 0) return x[q];
	if (q == xN - 1) return vpsrlq(x[q], r);
	return vor(vpsrlq(x[q], r), vpsllq(x[q+1], bitSize - r));
}

static inline void split(Unit a[2], Unit b[2], const Unit x[4])
{
	/*
		z = -0xd201000000010000
		L = z^2-1 = 0xac45a4010001a40200000000ffffffff
		r = L^2+L+1 = 0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001
		s=255
		v = 0xbe35f678f00fd56eb1fb72917b67f718
	*/
	static const uint64_t Lv[] = { 0x00000000ffffffff, 0xac45a4010001a402 };
	static const uint64_t vv[] = { 0xb1fb72917b67f718, 0xbe35f678f00fd56e };
	static const size_t n = 128 / mcl::UnitBitSize;
	Unit t[n*3];
	mcl::bint::mulNM(t, x, n*2, (const Unit*)vv, n);
	mcl::bint::shrT<n+1>(t, t+n*2-1, mcl::UnitBitSize-1); // >>255
	b[0] = t[0];
	b[1] = t[1];
	mcl::bint::mulT<n>(t, t, (const Unit*)Lv);
	mcl::bint::subT<n>(a, x, t);
}


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
		std::cout << "p=0x" << mp.get_str(16) << std::endl;
		std::cout << "R=0x" << mR.get_str(16) << std::endl;
		std::cout << "R2=0x" << mR2.get_str(16) << std::endl;
		printf("rp=0x%lx\n", rp);
	}
	void set(const mpz_class& _p)
	{
		mp = _p;
		mR = 1;
		mR = (mR << (W * N)) % mp;
		mR2 = (mR * mR) % mp;
		toArray<N>(v_, _p);
//		::put(v_, "v_");
		rp = mcl::bint::getMontgomeryCoeff(v_[0], W);
//		printf("rp=%zx\n", rp);
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

// 52bit x 52bit = 104 bit
Unit mul52bit(Unit *pH, Unit x, Unit y)
{
	Unit L, H;
	L = mcl::bint::mulUnit1(&H, x, y);
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

/*
	 |64   |64   |64   |64   |64    |64   |
	x|52:12|40:24|28:36|16:48|4:52:8|44:20|
    y|52|52   |52   |52   |52  |52|52  |20|
*/
void split52bit(Vec y[8], const Vec x[6])
{
	assert(&y != &x);
	y[0] = vand(x[0], vmask);
	y[1] = vand(vor(vpsrlq(x[0], 52), vpsllq(x[1], 12)), vmask);
	y[2] = vand(vor(vpsrlq(x[1], 40), vpsllq(x[2], 24)), vmask);
	y[3] = vand(vor(vpsrlq(x[2], 28), vpsllq(x[3], 36)), vmask);
	y[4] = vand(vor(vpsrlq(x[3], 16), vpsllq(x[4], 48)), vmask);
	y[5] = vand(vpsrlq(x[4], 4), vmask);
	y[6] = vand(vor(vpsrlq(x[4], 56), vpsllq(x[5], 8)), vmask);
	y[7] = vpsrlq(x[5], 44);
}

/*
	 |52|52   |52   |52   |52  |52|52  |20|
	x|52|12:40|24:28|36:16|48:4|52|8:44|20|
    y|64   |64   |64   |64   |64    |64
*/
void concat52bit(Vec y[6], const Vec x[8])
{
	assert(&y != &x);
	y[0] = vor(x[0], vpsllq(x[1], 52));
	y[1] = vor(vpsrlq(x[1], 12), vpsllq(x[2], 40));
	y[2] = vor(vpsrlq(x[2], 24), vpsllq(x[3], 28));
	y[3] = vor(vpsrlq(x[3], 36), vpsllq(x[4], 16));
	y[4] = vor(vor(vpsrlq(x[4], 48), vpsllq(x[5], 4)), vpsllq(x[6], 56));
	y[5] = vor(vpsrlq(x[6], 8), vpsllq(x[7], 44));
}

/*
	384bit = 6U (U=64)
	G1(=6U x 3(x, y, z)) x 8 => 8Ux8x3
*/
static CYBOZU_ALIGN(64) uint64_t g_pickUp[8] = {
	18*0, 18*1, 18*2, 18*3, 18*4, 18*5, 18*6, 18*7,
};
static const Vec& v_pickUp = *(const Vec*)g_pickUp;
void cvt6Ux3x8to8Ux8x3(Vec y[8*3], const Unit x[6*3*8])
{
	for (int j = 0; j < 3; j++) {
		Vec t[6];
		for (int i = 0; i < 6; i++) {
			t[i] = vpgatherqq(v_pickUp, x+j*6+i);
		}
		split52bit(&y[j*8], t);
	}
}

// EcM(=8Ux8x3) => G1(=6U x 3) x 8
void cvt8Ux8x3to6Ux3x8(Unit y[6*3*8], const Vec x[8*3])
{
	for (size_t j = 0; j < 3; j++) {
		Vec t[6];
		concat52bit(t, x+8*j);
		for (size_t i = 0; i < 6; i++) {
#if 1
			vpscatterqq(y+j*6+i, v_pickUp, t[i]);
#else
			const Unit *pt = (const Unit *)t;
			for (size_t k = 0; k < 8; k++) {
				y[j*6+k*18+i] = pt[k+i*8];
			}
#endif
		}
	}
}

// Fr x 8 = U4x8 => Vec(U8) x 4
void cvt4Ux8to8Ux4(Vec y[4], const Unit x[4*8])
{
	const size_t w = 4;
	for (size_t j = 0; j < M; j++) {
		for (size_t i = 0; i < w; i++) {
			((Unit *)y)[i*M+j] = x[j*w+i];
		}
	}
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

#if 0
// 7M+2S+9A+1 mul12
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
	mcl::ec::mul12(t2);
#else
	F::mul(t2, t2, E::b3_);
#endif
	F::mul(x3, t2, R.z);
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
#endif

struct Fp {
	static Fp one_;
	static Fp rw_;
	mpz_class v;
	Fp() : v(0) {}
	Fp(int _v) : v(g_mont.toMont(_v)) {}
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
	bool operator==(const Fp& rhs) const { return v == rhs.v; }
	bool operator!=(const Fp& rhs) const { return v != rhs.v; }
	Fp operator+(const Fp& rhs) const
	{
		Fp r;
		add(r, *this, rhs);
		return r;
	}
	Fp operator-(const Fp& rhs) const
	{
		Fp r;
		sub(r, *this, rhs);
		return r;
	}
	Fp operator*(const Fp& rhs) const
	{
		Fp r;
		mul(r, *this, rhs);
		return r;
	}
	friend std::ostream& operator<<(std::ostream& os, const Fp& x)
	{
		return os << x.get();
	}
	// z = x^y[yn]
	static void pow(Fp& z, const Fp& x, const Unit *y, size_t yn)
	{
		const int w = 4;
		const int tblN = 1<<w;
		const int mask = tblN-1;
		Fp tbl[tblN];
		tbl[0] = 1;
		tbl[1] = x;
		for (size_t i = 2; i < tblN; i++) {
			mul(tbl[i], tbl[i-1], x);
		}
		const size_t bitLen = sizeof(Unit)*8;
		const size_t jn = bitLen / w;
		z = tbl[0];
		for (size_t i = 0; i < yn; i++) {
			Unit v = y[yn-1-i];
			for (size_t j = 0; j < jn; j++) {
				for (int k = 0; k < w; k++) Fp::sqr(z, z);
				size_t idx = (v >> (bitLen-w - j*w)) & mask;
				Fp::mul(z, z, tbl[idx]);
			}
		}
	}
	static void inv(Fp& z, const Fp& x)
	{
		pow(z, x, g_mpM2, sizeof(g_mpM2)/sizeof(g_mpM2[0]));
	}
	void put(const char *msg = nullptr, int base = 16) const
	{
		if (msg) printf("%s ", msg);
		printf("%s\n", get().get_str(base).c_str());
	}
	void putRaw(const char *msg = nullptr) const
	{
		if (msg) printf("%s ", msg);
		printf("%s\n", v.get_str(16).c_str());
	}
};
Fp Fp::one_;
Fp Fp::rw_;

struct Ec {
	typedef ::Fp Fp;
	static const int a_ = 0;
	static const int b_ = 4;
	static Fp b3_;
	static const int specialB_ = mcl::ec::local::Plus4;
	static Ec zero_;
	Fp x, y, z;
	static void add(Ec& z, const Ec& x, const Ec& y)
	{
		mcl::ec::addCTProj(z, x, y);
	}
	static void dbl(Ec& z, const Ec& x)
	{
		mcl::ec::dblCTProj(z, x);
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
	// proj
	bool operator==(const Ec& rhs) const
	{
		return x * rhs.z == rhs.x * z && y * rhs.z == rhs.y * z;
	}
	bool operator!=(const Ec& rhs) const { return !operator==(rhs); }
	bool isValid() const
	{
		return y * y * z == x * x * x + z * z * z * b_;
	}
	static void mul(Ec& Q, const Ec& P, const Unit *y, size_t yn)
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
		const size_t jn = bitLen / w;
		Q = tbl[0];
		for (size_t i = 0; i < yn; i++) {
			Unit v = y[yn-1-i];
			for (size_t j = 0; j < jn; j++) {
				for (int k = 0; k < w; k++) Ec::dbl(Q, Q);
				size_t idx = (v >> (bitLen-w - j*w)) & mask;
				Ec::add(Q, Q, tbl[idx]);
			}
		}
	}
	static void mul(Ec& Q, const Ec& P, Unit y)
	{
		mul(Q, P, &y, 1);
	}
	void normalize()
	{
		if (z.v == 0) return;
		Fp r;
		Fp::inv(r, z);
		Fp::mul(x, x, r);
		Fp::mul(y, y, r);
		z = Fp::one_;
	}
	void put(const char *msg = nullptr) const
	{
		if (msg) printf("%s\n", msg);
		x.put("x");
		y.put("y");
		z.put("z");
	}
	void putRaw(const char *msg = nullptr) const
	{
		if (msg) printf("%s\n", msg);
		x.putRaw("x");
		y.putRaw("y");
		z.putRaw("z");
	}
	static void mulLambda(Ec& Q, const Ec& P)
	{
		Fp::mul(Q.x, P.x, Fp::rw_);
		Q.y = P.y;
		Q.z = P.z;
	}
	static void mulGLV(Ec& Q, const Ec& P, const Unit y[4])
	{
		Unit a[2], b[2];
		Ec T;
		mulLambda(T, P);
		split(a, b, y);
		mul(Q, P, a, 2);
		mul(T, T, b, 2);
		add(Q, Q, T);
	}
};

Fp Ec::b3_;
Ec Ec::zero_;

struct FpM {
	Vec v[N];
	static FpM one_;
	static FpM rawOne_;
	static FpM rw_;
	static FpM mR2_;
	static FpM m64to52_;
	static FpM m52to64_;
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
		mpz_class r = g_mont.toMont(x);
		Unit rv[N];
		toArray<N>(rv, r);
		::set(v, i, rv);
	}
	void set(const mpz_class& x)
	{
		mpz_class r = g_mont.toMont(x);
		Unit rv[N];
		toArray<N>(rv, r);
		for (size_t i = 0; i < M; i++) {
			::set(v, i, rv);
		}
	}
	void toMont(FpM& x) const
	{
		mul(x, *this, mR2_);
	}
	void fromMont(const FpM &x)
	{
		mul(*this, x, rawOne_);
	}
	mpz_class getRaw(size_t i) const
	{
		Unit x[N];
		::get(x, v, i);
		return fromArray<N>(x);
	}
	mpz_class get(size_t i) const
	{
		mpz_class r = getRaw(i);
		return g_mont.fromMont(r);
	}
	bool operator==(const FpM& rhs) const
	{
		for (size_t i = 0; i < N; i++) {
			if (memcmp(&v[i], &rhs.v[i], sizeof(Vec)) != 0) return false;
		}
		return true;
	}
	bool operator!=(const FpM& rhs) const { return !operator==(rhs); }
	void put(const char *msg = nullptr, int base = 16) const
	{
		if (msg) printf("%s\n", msg);
		for (size_t i = 0; i < M; i++) {
			printf("% 2zd %s\n", i, get(i).get_str(base).c_str());
		}
	}
	void putRaw(const char *msg = nullptr) const
	{
		if (msg) printf("%s\n", msg);
		for (size_t i = 0; i < M; i++) {
			printf("% 2zd %s\n", i, getRaw(i).get_str(16).c_str());
		}
	}
	friend std::ostream& operator<<(std::ostream& os, const FpM& x)
	{
		for (size_t i = 0; i < N; i++) {
			os << i << ' ' << x.get(i) << '\n';
		}
		return os;
	}
	static void pow(FpM& z, const FpM& x, const Vec *y, size_t yn)
	{
		const int w = 4;
		assert(w == 4);
		const int tblN = 1<<w;
		FpM tbl[tblN];
		tbl[0] = one_;
		tbl[1] = x;
		for (size_t i = 2; i < tblN; i++) {
			mul(tbl[i], tbl[i-1], x);
		}
		const size_t bitLen = sizeof(Unit)*8;
		const size_t jn = bitLen / w;
		z = tbl[0];
		for (size_t i = 0; i < yn; i++) {
			const Vec& v = y[yn-1-i];
			for (size_t j = 0; j < jn; j++) {
				for (int k = 0; k < w; k++) FpM::sqr(z, z);
				Vec idx = vand(vpsrlq(v, bitLen-w-j*w), g_vmask4);
				idx = vpsllq(idx, 6); // 512 B = 64 Unit
				idx = vadd(idx, g_offset);
				FpM t;
				for (size_t k = 0; k < N; k++) {
					t.v[k] = vpgatherqq(idx, &tbl[0].v[k]);
				}
				mul(z, z, t);
			}
		}
	}
	static void inv(FpM& z, const FpM& x)
	{
		pow(z, x, g_vmpM2, 6);
	}
};

FpM FpM::one_;
FpM FpM::rawOne_;
FpM FpM::rw_;
FpM FpM::mR2_;
FpM FpM::m64to52_;
FpM FpM::m52to64_;

struct EcM {
	typedef FpM Fp;
	static const int a_ = 0;
	static const int b_ = 4;
	static const int specialB_ = mcl::ec::local::Plus4;
	static const int w = 4;
	static const int tblN = 1<<w;
	static const size_t bitLen = sizeof(Unit)*8;
	static FpM b3_;
	static EcM zero_;
	FpM x, y, z;
	static void add(EcM& z, const EcM& x, const EcM& y)
	{
		mcl::ec::addCTProj(z, x, y);
	}
	static void dbl(EcM& z, const EcM& x)
	{
		mcl::ec::dblCTProj(z, x);
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
		x.set(v.x.get(), i);
		y.set(v.y.get(), i);
		z.set(v.z.get(), i);
	}
	Ec get(size_t i) const
	{
		Ec P;
		P.x.set(x.get(i));
		P.y.set(y.get(i));
		P.z.set(z.get(i));
		return P;
	}
	void set(const Ec& v)
	{
		for (size_t i = 0; i < M; i++) {
			set(v, i);
		}
	}
	void set_naive(const Ec v[M])
	{
		for (size_t i = 0; i < M; i++) {
			set(v[i], i);
		}
	}
	void setArray(const Unit a[6*3*M])
	{
		cvt6Ux3x8to8Ux8x3(x.v, a);
	}
	void getArray(Unit a[6*3*M]) const
	{
		cvt8Ux8x3to6Ux3x8(a, x.v);
	}
	void set(const Ec v[M])
	{
		Unit a[6*3*M];
		for (size_t i = 0; i < M; i++) {
			mcl::gmp::getArray(&a[6*3*i+6*0], 6, v[i].x.v);
			mcl::gmp::getArray(&a[6*3*i+6*1], 6, v[i].y.v);
			mcl::gmp::getArray(&a[6*3*i+6*2], 6, v[i].z.v);
		}
		setArray(a);
	}
	void getEc(Ec v[M]) const
	{
		Unit a[6*3*M];
		getArray(a);
		for (size_t i = 0; i < M; i++) {
			mcl::gmp::setArray(v[i].x.v, &a[6*3*i+6*0], 6);
			mcl::gmp::setArray(v[i].y.v, &a[6*3*i+6*1], 6);
			mcl::gmp::setArray(v[i].z.v, &a[6*3*i+6*2], 6);
		}
	}
	void setG1(const mcl::bn::G1 v[M])
	{
#if 1
		setArray((const Unit*)v);
		FpM::mul(x, x, FpM::m64to52_);
		FpM::mul(y, y, FpM::m64to52_);
		FpM::mul(z, z, FpM::m64to52_);
#else
		Unit a[6*3*M];
		const Unit *src = (const Unit *)v;
		for (size_t i = 0; i < M*3; i++) {
			mcl::bn::Fp::getOp().fromMont(a+i*6, src+i*6);
		}
		setArray(a);
		x.toMont(x);
		y.toMont(y);
		z.toMont(z);
#endif
		mcl::ec::JacobiToProj(*this, *this);
	}
	void getG1(mcl::bn::G1 v[M]) const
	{
		EcM T;
		mcl::ec::ProjToJacobi(T, *this);
#if 1
		FpM::mul(T.x, T.x, FpM::m52to64_);
		FpM::mul(T.y, T.y, FpM::m52to64_);
		FpM::mul(T.z, T.z, FpM::m52to64_);
		T.getArray((Unit*)v);
#else
		T.x.fromMont(T.x);
		T.y.fromMont(T.y);
		T.z.fromMont(T.z);
		Unit a[6*3*M];
		T.getArray(a);
		Unit *dst = (Unit *)v;
		for (size_t i = 0; i < M*3; i++) {
			mcl::bn::Fp::getOp().toMont(dst+i*6, a+i*6);
		}
#endif
	}
	void put(const char *msg = nullptr) const
	{
		if (msg) printf("%s\n", msg);
		x.put("x");
		y.put("y");
		z.put("z");
	}
	void putRaw(const char *msg = nullptr) const
	{
		if (msg) printf("%s\n", msg);
		x.putRaw("x");
		y.putRaw("y");
		z.putRaw("z");
	}
	void normalize()
	{
		FpM r;
		FpM::inv(r, z);
		FpM::mul(x, x, r);
		FpM::mul(y, y, r);
		z = FpM::one_;
	}
	static void makeTable(EcM *tbl, const EcM& P)
	{
		tbl[0].clear();
		tbl[1] = P;
		for (size_t i = 2; i < tblN; i++) {
			add(tbl[i], tbl[i-1], P);
		}
	}
	void gather(const EcM *tbl, Vec idx)
	{
		idx = vmulL(idx, g_vi192, g_offset);
		for (size_t i = 0; i < N; i++) {
			x.v[i] = vpgatherqq(idx, &tbl[0].x.v[i]);
			y.v[i] = vpgatherqq(idx, &tbl[0].y.v[i]);
			z.v[i] = vpgatherqq(idx, &tbl[0].z.v[i]);
		}
	}
	void scatter(EcM *tbl, Vec idx) const
	{
		idx = vmulL(idx, g_vi192, g_offset);
		for (size_t i = 0; i < N; i++) {
			vpscatterqq(&tbl[0].x.v[i], idx, x.v[i]);
			vpscatterqq(&tbl[0].y.v[i], idx, y.v[i]);
			vpscatterqq(&tbl[0].z.v[i], idx, z.v[i]);
		}
	}

	static void mul(EcM& Q, const EcM& P, const Vec *y, size_t yn)
	{
		EcM tbl[tblN];
		makeTable(tbl, P);
		const size_t jn = bitLen / w;
		Q = tbl[0];
		for (size_t i = 0; i < yn; i++) {
			const Vec& v = y[yn-1-i];
			for (size_t j = 0; j < jn; j++) {
				for (int k = 0; k < w; k++) EcM::dbl(Q, Q);
				Vec idx = vand(vpsrlq(v, bitLen-w-j*w), g_vmask4);
				EcM T;
				T.gather(tbl, idx);
				add(Q, Q, T);
			}
		}
	}
	static void mulLambda(EcM& Q, const EcM& P)
	{
		FpM::mul(Q.x, P.x, FpM::rw_);
		Q.y = P.y;
		Q.z = P.z;
	}
	static void mulGLV(EcM& Q, const EcM& P, const Vec y[4])
	{
		Vec a[2], b[2];
		EcM tbl1[tblN], tbl2[tblN];
		makeTable(tbl1, P);
		for (size_t i = 0; i < tblN; i++) {
			mulLambda(tbl2[i], tbl1[i]);
		}
		const Unit *src = (const Unit*)y;
		Unit *pa = (Unit*)a;
		Unit *pb = (Unit*)b;
		for (size_t i = 0; i < M; i++) {
			Unit buf[4] = { src[i+M*0], src[i+M*1], src[i+M*2], src[i+M*3] };
			Unit aa[2], bb[2];
			split(aa, bb, buf);
			pa[i+M*0] = aa[0]; pa[i+M*1] = aa[1];
			pb[i+M*0] = bb[0]; pb[i+M*1] = bb[1];
		}
#if 1
		const size_t jn = bitLen / w;
		const size_t yn = 2;
		Q.clear();
		for (size_t i = 0; i < yn; i++) {
			const Vec& v1 = a[yn-1-i];
			const Vec& v2 = b[yn-1-i];
			for (size_t j = 0; j < jn; j++) {
				for (int k = 0; k < w; k++) EcM::dbl(Q, Q);
				EcM T;
				Vec idx;
				idx = vand(vpsrlq(v1, bitLen-w-j*w), g_vmask4);
				T.gather(tbl1, idx);
				add(Q, Q, T);
				idx = vand(vpsrlq(v2, bitLen-w-j*w), g_vmask4);
				T.gather(tbl2, idx);
				add(Q, Q, T);
			}
		}
#else
		mul(Q, P, a, 2);
		mul(T, T, b, 2);
		add(Q, Q, T);
#endif
	}
};

FpM EcM::b3_;
EcM EcM::zero_;

void init(Montgomery& mont)
{
	const char *pStr = "1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab";
	const char *rStr = "73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001";
	g_mp.set_str(pStr, 16);
	g_mr.set_str(rStr, 16);
	mont.set(g_mp);
	toArray<N>(g_p, g_mp);
	toArray<6, 64>(g_mpM2, g_mp-2);
	expand(vmask, g_mask);
	expandN(vpN, g_mp);
	expand(vrp, mont.rp);
	for (int i = 0; i < 6; i++) {
		expand(g_vmpM2[i], g_mpM2[i]);
	}
	expand(g_vmask4, getMask(4));
//	put(g_vmask4, "g_vmask4");
	for (int i = 0; i < 8; i++) {
		((Unit*)&g_offset)[i] = i;
	}
	expand(g_vi192, 192);
	Fp::one_.set(1);
	expandN(FpM::one_.v, Fp(1).v);
	expandN(FpM::rawOne_.v, mpz_class(1));
	expandN(FpM::mR2_.v, g_mont.mR2);
	{
		mpz_class t;
		FpM::m64to52_.set(mpz_class(0x100000000));
		FpM::inv(FpM::m52to64_, FpM::m64to52_);
	}
	Ec::init(mont);
	EcM::init(mont);
	Ec::zero_.set(0, 1, 0);
	EcM::zero_.set(Ec::zero());
	const char *rwStr = "1a0111ea397fe699ec02408663d4de85aa0d857d89759ad4897d29650fb85f9b409427eb4f49fffd8bfd00000000aaac";
	mpz_class rw;
	rw.set_str(rwStr, 16);
	Fp::rw_.set(rw);
	FpM::rw_.set(rw);

	g_mx.set_str("17f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb", 16);
	g_my.set_str("08b3f481e3aaa0f1a09e30ed741d8ae4fcf5e095d5d00af600db18cb2c04b3edd03cc744a2888ae40caa232946c5e7e1", 16);
	g_mr.set_str("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001", 16);
//	printf("(mx, my) is on %d\n", (g_mx * g_mx * g_mx + 4 - g_my * g_my) % g_mp == 0);
}

template<typename T>
void assertEq(const T& a, const T& b, const char *msg = nullptr)
{
	if (a != b) {
		printf("ERR ");
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
			puts("ERR mulUnit");
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
		assertEq(P.x.get(i), Q[i].x.get(), "x");
		assertEq(P.y.get(i), Q[i].y.get(), "y");
		assertEq(P.z.get(i), Q[i].z.get(), "z");
	}
}

void powTest()
{
	{
		Unit yv[4] = {};
		Fp x = 100, y, z, w = x;
		for (int i = 1; i < 100; i++) {
			yv[0] = i;
			Fp::pow(z, x, yv, 4);
			assertEq(z, w);
			Fp::mul(w, w, x);
		}
		puts("pow ok");

		for (int i = 0; i < 100; i++) {
			Fp::add(x, x, x);
			Fp::inv(y, x);
			Fp::mul(z, x, y);
			assertEq(z.get(), mpz_class(1));
		}
		puts("inv ok");
	}
	{
		Fp x[8], z[8];
		Unit y[8];
		FpM xm, zm;
		Vec ym;
		for (int i = 0; i < 8; i++) {
			x[i].set(i+0x98765432);
			y[i] = i+0x12345678;
			xm.set(x[i].get(), i);
		}
		memcpy(&ym, y, sizeof(y));
		for (int i = 0; i < 8; i++) {
			Fp::pow(z[i], x[i], &y[i], 1);
		}
		FpM::pow(zm, xm, &ym, 1);

		for (int i = 0; i < 8; i++) {
			assertEq(z[i].get(), zm.get(i));
		}
	}
	puts("FpM::pow ok");
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
		Ec t = P1[i];
		t.normalize();
		if (t != P1[i]) {
			puts("normalize err");
			P1[i].put("P1");
			t.put("t");
			exit(1);
		}
	}
	P2.set(P1);
	R2.set_naive(P1);
	if (memcmp(&P2, &R2, sizeof(P2)) != 0) {
		P2.put("P2");
		R2.put("R2");
		exit(1);
	}
	cmpEc(P2, P1, "P");
	{
		Vec y[6];
		for (int i = 0; i < 6; i++) {
			expand(y[i], g_mpM2[i]);
		}
		FpM x = P2.x, z;
		FpM::inv(z, x);
		FpM::mul(z, z, x);
		for (int i = 0; i < 8; i++) {
			assertEq(z.get(i), mpz_class(1));
		}
		puts("vpow ok");
	}

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
	CYBOZU_BENCH_C("FpM::inv", 1000, FpM::inv, R2.x, R2.x);
}

void GLVtest()
{
	puts("GLVtest");
	Ec P, Q, R;
	P.x.set(mpz_class("13400cd4fe26471eef602b91432ac4180e12519b68e7b7efa081fc8a20654e215ef7eb439a622e21466a3dd6add55fac", 16));
	P.y.set(mpz_class("b81cd9a01198e1ea3ba3d94aaf4036750f84537fa5d2a9a6fa917b8a1d264b7c513f4dcef866142bd33094107404452", 16));
//	P.x.set(g_mx);
//	P.y.set(g_my);
	P.z.set(1);
	for (int i = 0; i < 10; i++) {
		Ec::mulLambda(Q, P);
		Ec::mul(P, P, g_L, 2);
		if (Q != P) {
			puts("ERR");
		}
	}
	mpz_class y("2ac00f2c9af814438db241461ec7825ed88d00b0951049aa1b5116e6dca345ea", 16);
	Unit ya[4];
	toArray<4, 64>(ya, y);
	Ec::mul(Q, P, ya, 4);
	Q.normalize();
	Ec::mulGLV(R, P, ya);
	if (Q != R) {
		puts("ERR GLV");
		Q.put("Q");
		R.put("R");
		exit(1);
	}
	EcM P1, Q1;
	Vec vy[4];
	for (size_t i = 0; i < 4; i++) {
		((Unit*)vy)[i*M] = ya[i];
	}
	memset(&P1, 0, sizeof(P1));
	memset(&Q1, 0, sizeof(Q1));
	P1.set(P, 0);
	EcM::mulGLV(Q1, P1, vy);
	if (Q != Q1.get(0)) {
		puts("ERR EcM::mulGLV");
		exit(1);
	}
}

void mulTest()
{
	puts("mulTest");
	Ec P1[M], Q1[M];
	EcM P2, Q2, Q3;
	const int w = 4;
	Unit y[w*M];
	Vec yv[w];
	P1[0].x.set(g_mx);
	P1[0].y.set(g_my);
	P1[0].z.set(1);
	for (size_t i = 1; i < M; i++) {
		Ec::dbl(P1[i], P1[i-1]);
		Ec T1, T2;
		Ec::mulLambda(T1, P1[i]);
		Ec::mul(T2, P1[i], g_L, 2);
		if (T1 != T2) {
			printf("ERR %zd\n", i);
			T1.normalize();
			T2.normalize();
			P1[i].put("P");
			T1.put("T1");
			T2.put("T2");
			exit(1);
		}
	}
	P2.set(P1);
	{
		cybozu::XorShift rg;
		for (size_t i = 0; i < M; i++) {
			(*(mcl::bn::Fr*)&y[i*w]).setByCSPRNG(rg);
		}
	}
//	for (size_t i = 0; i < w*M; i++) {
//		y[i] = i * 0x123456 + 0x345678;
//	}
	cvt4Ux8to8Ux4(yv, y);
	for (size_t i = 0; i < M; i++) {
		Ec::mul(Q1[i], P1[i], y+i*w, w);
		Ec t;
		Ec::mulGLV(t, P1[i], y+i*w);
		if (Q1[i] != t) {
			printf("ERRRR %zd\n", i);
			Q1[i].put("Q1");
			t.put("t");
			exit(1);
		}
	}
	EcM::mul(Q2, P2, yv, w);
	EcM::mulGLV(Q3, P2, yv);

	for (size_t i = 0; i < M; i++) {
		if (Q1[i] != Q2.get(i)) {
			printf("%zd\n", i);
			Q1[i].put("Q1");
			Q2.get(i).put("Q2");
			exit(1);
		}
		if (Q2.get(i) != Q3.get(i)) {
			printf("ERR %zd\n", i);
			Q2.get(i).put("Q2");
			Q3.get(i).put("Q3");
			exit(1);
		}
	}
	CYBOZU_BENCH_C("EcM::mul(2)", 10000, EcM::mul, P2, P2, yv, 2);
	CYBOZU_BENCH_C("EcM::mul(4)", 10000, EcM::mul, P2, P2, yv, w);
	CYBOZU_BENCH_C("EcM::mulGLV", 10000, EcM::mulGLV, P2, P2, yv);
	{
		using namespace mcl;
		const bn::Fr& ya = *(const bn::Fr*)y;
		bn::G1 xa;
		bn::hashAndMapToG1(xa, "abc", 3);
		CYBOZU_BENCH_C("G1::mul", 10000, bn::G1::mul, xa, xa, ya);
	}
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

void gatherTest()
{
	const int N = 64;
	Unit tbl[N], xv[8];
	for (int i = 0; i < N; i++) {
		tbl[i] = 1000+i;
	}
	for (int i = 0; i < 8; i++) {
		xv[i] = i*4;
	}
	Vec z, x;
	memcpy(&x, xv, sizeof(xv));
	z = vpgatherqq(x, tbl);
	Unit zv[8];
	memcpy(zv, &z, sizeof(z));
	for (int i = 0; i < 8; i++) {
		assertEq(zv[i], tbl[xv[i]]);
	}
}

void split52bitTest()
{
	puts("split52bitTest");
	Vec x[6], y[8], z[6];
	Unit *px = (Unit *)x;
	Unit *py = (Unit *)y;
	for (int i = 0; i < 384; i++) {
		memset(x, 0, sizeof(x));
		memset(y, 0, sizeof(y));
		int q = i / 64;
		int r = i % 64;
		px[q*M] = Unit(1) << r;
		split52bit(y, x);
		int q2 = i / 52;
		int r2 = i % 52;
		if (py[q2*M] != (Unit(1) << r2)) {
			printf("err i=%d\n", i);
			exit(1);
		}
		for (int j = 0; j < 8; j++) {
			if (j != q2 && py[j*M] != 0) {
				printf("err2 i=%d j=%d\n", i, j);
				exit(1);
			}
		}
		concat52bit(z, y);
		if (memcmp(x, z, sizeof(x)) != 0) {
			puts("err3");
			exit(1);
		}
	}
	cybozu::XorShift rg;
	for (size_t i = 0; i < 6*M; i++) {
		px[i] = rg.get64();
	}
	split52bit(y, x);
	concat52bit(z, y);
	if (memcmp(x, z, sizeof(x)) != 0) {
		puts("err4");
		exit(1);
	}
}

void cvtTest()
{
	const size_t n = 6*3*8;
	Unit x[n], z[n];
	Vec y[8*3];
	cybozu::XorShift rg;
	for (size_t i = 0; i < n; i++) {
		x[i] = rg.get64();
	}
	cvt6Ux3x8to8Ux8x3(y, x);
	cvt8Ux8x3to6Ux3x8(z, y);
	if (memcmp(x, z, sizeof(x)) != 0) {
		puts("cvtTest err");
		for (size_t j = 0; j < n/6; j++) {
			printf("%zd\n", j);
			printf("x ");
			for (size_t i = 0; i < 6; i++) {
				printf("%016lx ", x[j*6+i]);
			}
			printf("\nz ");
			for (size_t i = 0; i < 6; i++) {
				printf("%016lx ", z[j*6+i]);
			}
			printf(" %c\n", memcmp(&x[j*6], &z[j*6], sizeof(Unit)*6) == 0 ? 'o' : 'x');
		}
		exit(1);
	}
	CYBOZU_BENCH_C("cvt6Ux3x8to8Ux8x3", C, cvt6Ux3x8to8Ux8x3, y, x);
	CYBOZU_BENCH_C("cvt8Ux8x3to6Ux3x8", C, cvt8Ux8x3to6Ux3x8, z, y);
}

void reduceSum(mcl::bn::G1& Q, const EcM& P)
{

	mcl::bn::G1 z[8];
	P.getG1(z);
	Q = z[0];
	for (int i = 1; i < 8; i++) {
		Q += z[i];
	}
}

void cvtFr8toVec4(Vec yv[4], const mcl::bn::Fr y[8])
{
	Unit ya[4*8];
	for (size_t i = 0; i < 8; i++) {
		mcl::bn::Fr::getOp().fromMont(ya+i*4, y[i].getUnit());
	}
	cvt4Ux8to8Ux4(yv, ya);
}

void mulVecAVX512_naive(mcl::bn::G1& P, const mcl::bn::G1 *x, const mcl::bn::Fr *y, size_t n)
{
	assert(n % 8 == 0);
	EcM R;
	R.clear();
	for (size_t i = 0; i < n; i += 8) {
		Vec yv[4];
		cvtFr8toVec4(yv, y+i);
		EcM T, X;
		X.setG1(x+i);
		EcM::mulGLV(T, X, yv);
		EcM::add(R, R, T);
	}
	reduceSum(P, R);
}

inline void *AlignedMalloc(size_t size, size_t alignment = 64)
{
#ifdef __MINGW32__
	return __mingw_aligned_malloc(size, alignment);
#elif defined(_WIN32)
	return _aligned_malloc(size, alignment);
#else
	void *p;
	int ret = posix_memalign(&p, alignment, size);
	return (ret == 0) ? p : 0;
#endif
}


// xVec[n], yVec[n * maxBitSize/64]
void mulVecAVX512_inner(mcl::bn::G1& P, const EcM *xVec, const Vec *yVec, size_t n, size_t maxBitSize)
{
	size_t c = mcl::ec::argminForMulVec(n);
	size_t tblN = 1 << c;
	EcM *tbl = (EcM*)AlignedMalloc(sizeof(EcM) * tblN);
	const size_t yn = maxBitSize / 64;
	const size_t winN = (maxBitSize + c-1) / c;
	EcM *win = (EcM*)AlignedMalloc(sizeof(EcM) * winN);

	const Vec m = vpbroadcastq(tblN-1);
	for (size_t w = 0; w < winN; w++) {
		for (size_t i = 0; i < tblN; i++) {
			tbl[i].clear();
		}
		for (size_t i = 0; i < n; i++) {
			Vec v = getUnitAt(yVec+i*yn, yn, c*w);
			v = vand(v, m);
			EcM T;
			T.gather(tbl, v);
			EcM::add(T, T, xVec[i]);
			T.scatter(tbl, v);
		}
		EcM sum = tbl[tblN - 1];
		win[w] = sum;
		for (size_t i = 1; i < tblN - 1; i++) {
			EcM::add(sum, sum, tbl[tblN - 1- i]);
			EcM::add(win[w], win[w], sum);
		}
	}
	EcM T = win[winN - 1];
	for (size_t w = 1; w < winN; w++) {
		for (size_t i = 0; i < c; i++) {
			EcM::dbl(T, T);
		}
		EcM::add(T, T, win[winN - 1- w]);
	}
	reduceSum(P, T);
	free(win);
	free(tbl);
}

void mulVecAVX512(mcl::bn::G1& P, const mcl::bn::G1 *x, const mcl::bn::Fr *y, size_t n)
{
	assert(n % 8 == 0);
	const size_t n8 = n/8;
#if 1
	EcM *xVec = (EcM*)AlignedMalloc(sizeof(EcM) * n8 * 2);
	for (size_t i = 0; i < n8; i++) {
		xVec[i*2].setG1(x+i*8);
		EcM::mulLambda(xVec[i*2+1], xVec[i*2]);
	}
	Vec *yVec = (Vec*)AlignedMalloc(sizeof(Vec) * n8 * 4);
	Unit *py = (Unit*)yVec;
	for (size_t i = 0; i < n8; i++) {
		for (size_t j = 0; j < 8; j++) {
			Unit ya[4];
			mcl::bn::Fr::getOp().fromMont(ya, y[i*8+j].getUnit());
			Unit a[2], b[2];
			split(a, b, ya);
			py[j+0] = a[0];
			py[j+8] = a[1];
			py[j+16] = b[0];
			py[j+24] = b[1];
		}
		py += 32;
	}
	mulVecAVX512_inner(P, xVec, yVec, n8*2, 128);
#else
	EcM *xVec = (EcM*)AlignedMalloc(sizeof(EcM) * n8);
	for (size_t i = 0; i < n8; i++) {
		xVec[i].setG1(x+i*8);
	}
	Vec *yVec = (Vec*)AlignedMalloc(sizeof(Vec) * n8 * 4);
	for (size_t i = 0; i < n8; i++) {
		cvtFr8toVec4(yVec+i*4, y+i*8);
	}
	mulVecAVX512_inner(P, xVec, yVec, n8, 256);
#endif
	free(yVec);
	free(xVec);
}

void mulVec_naive(mcl::bn::G1& P, const mcl::bn::G1 *x, const mcl::bn::Fr *y, size_t n)
{
#if 0
	using namespace mcl::bn;
	G1::mul(P, x[0], y[0]);
	for (size_t i = 1; i < n; i++) {
		G1 T;
		G1::mul(T, x[i], y[i]);
		P += T;
	}
#else
	size_t c = mcl::ec::argminForMulVec(n);
	size_t tblN = (1 << c) - 0;
	mcl::bn::G1 *tbl = (mcl::bn::G1*)CYBOZU_ALLOCA(sizeof(mcl::bn::G1) * tblN);
	const size_t maxBitSize = 256;
	const size_t winN = (maxBitSize + c-1) / c;
	mcl::bn::G1 *win = (mcl::bn::G1*)CYBOZU_ALLOCA(sizeof(mcl::bn::G1) * winN);

	Unit *yVec = (Unit*)CYBOZU_ALLOCA(sizeof(mcl::bn::Fr) * n);
	for (size_t i = 0; i < n; i++) {
		mcl::bn::Fr::getOp().fromMont(yVec+i*4, y[i].getUnit());
	}
	for (size_t w = 0; w < winN; w++) {
		for (size_t i = 0; i < tblN; i++) {
			tbl[i].clear();
		}
		for (size_t i = 0; i < n; i++) {
			Unit v = mcl::fp::getUnitAt(yVec+i*4, 4, c * w) & (tblN-1);
			tbl[v] += x[i];
		}
		mcl::bn::G1 sum = tbl[tblN-1];
		win[w] = sum;
		for (size_t i = 1; i < tblN-1; i++) {
			sum += tbl[tblN - 1 - i];
			win[w] += sum;
		}
	}
	P.clear(); // remove a wrong gcc warning
	P = win[winN - 1];
	for (size_t w = 1; w < winN; w++) {
		for (size_t i = 0; i < c; i++) {
			mcl::bn::G1::dbl(P, P);
		}
		P += win[winN - 1 - w];
	}
#endif
}

void mtTest(size_t n, bool onlyBench)
{
	using namespace mcl::bn;
	const int C = 10;
	cybozu::XorShift rg;
	std::vector<G1> Pvec(n);
	std::vector<Fr> xVec(n);
	hashAndMapToG1(Pvec[0], "abc", 3);
	for (size_t i = 1; i < n; i++) {
		G1::add(Pvec[i], Pvec[i-1], Pvec[0]);
	}
	for (size_t i = 0; i < n; i++) {
		xVec[i].setByCSPRNG(rg);
	}
	{
		EcM Q;
		G1 R[8];
		Q.setG1(Pvec.data());
		Q.getG1(R);
		for (int i = 0; i < 8; i++) {
			if (Pvec[i] != R[i]) {
				printf("ERR %d\n", i);
				printf("P %s\n", Pvec[i].getStr(16|mcl::IoEcProj).c_str());
				printf("R %s\n", R[i].getStr(16|mcl::IoEcProj).c_str());
				exit(1);
			}
		}
	}
	G1 P1, P2, P3, P4;
	if (onlyBench) {
		CYBOZU_BENCH_C("mulVecAVX512", C, mulVecAVX512, P4, Pvec.data(), xVec.data(), n);
		return;
	}
	G1::mulVec(P1, Pvec.data(), xVec.data(), n);
	mulVec_naive(P2, Pvec.data(), xVec.data(), n);
	mulVecAVX512_naive(P3, Pvec.data(), xVec.data(), n);
	mulVecAVX512(P4, Pvec.data(), xVec.data(), n);
	if (P1 != P2) {
		puts("err mulVec_naive");
		printf("P1=%s\n", P1.getStr(16).c_str());
		printf("P2=%s\n", P2.getStr(16).c_str());
		exit(1);
	}
	if (P1 != P3) {
		puts("err mulVecAVX512_naive");
		printf("P1=%s\n", P1.getStr(16|mcl::IoEcProj).c_str());
		printf("P3=%s\n", P3.getStr(16|mcl::IoEcProj).c_str());
		exit(1);
	}
#if 1
	if (P1 != P4) {
		puts("err mulVecAVX512");
		printf("P1=%s\n", P1.getStr(16|mcl::IoEcProj).c_str());
		printf("P4=%s\n", P4.getStr(16|mcl::IoEcProj).c_str());
		exit(1);
	}
#endif
	CYBOZU_BENCH_C("G1::mulVec", C, G1::mulVec, P1, Pvec.data(), xVec.data(), n);
//	CYBOZU_BENCH_C("mulVec_naive", C, mulVec_naive, P2, Pvec.data(), xVec.data(), n);
//	CYBOZU_BENCH_C("mulVecAVX512_naive", C, mulVecAVX512_naive, P3, Pvec.data(), xVec.data(), n);
	CYBOZU_BENCH_C("mulVecAVX512", C, mulVecAVX512, P4, Pvec.data(), xVec.data(), n);
}

int main(int argc, char *argv[])
{
	size_t xn;
	bool onlyBench;
	cybozu::Option opt;
	opt.appendOpt(&xn, 8192, "n", ":# of elem");
	opt.appendBoolOpt(&onlyBench, "b", ": benchmark");
	opt.appendHelp("h");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	printf("xn=%zd\n", xn);
	mcl::bn::initPairing(mcl::BLS12_381);
	init(g_mont);

	mtTest(xn, onlyBench);
	if (onlyBench) return 0;

	g_mont.put();

	const mpz_class tbl[] = {
		0xaabbccdd, 0x11223344, 0, 1, 2, g_mask-1, g_mask, g_mask+1, g_mp-1, g_mp>>2, 0x12345, g_mp-0x1111,
	};
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
	cvtTest();
	split52bitTest();
	gatherTest();
	miscTest();
	powTest();
	GLVtest();
	mulTest();
	ecTest();
	puts("ok");
}
