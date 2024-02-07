/*
clang++-15 -Ofast vec-op.cpp -lgmp -lgmpxx -I ../cybozulib/include/ -mavx512f -mavx512ifma -Wall -Wextra && ./a.out
*/
#include <stdint.h>
#include <stdio.h>
#include <gmpxx.h>
#include <iostream>
#include <cybozu/xorshift.hpp>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef uint64_t Unit;
const size_t S = sizeof(Unit)*8-1; // 63
const size_t W = 52;
const size_t N = 8; // = ceil(384/52)
const uint64_t mask = (Unit(1)<<W) - 1;

static mpz_class mp;

// split into 52 bits
static Unit p[N];

typedef __m512i Vec;
typedef __mmask8 Vmask;
const size_t M = sizeof(Vec) / sizeof(Unit);

void put(const Vec& v, const char *msg = nullptr)
{
	if (msg) printf("%s ", msg);
	const Unit *p = (const Unit*)&v;
	for (size_t i = 0; i < M; i++) {
		printf("%014lx ", p[M-1-i]);
	}
	printf("\n");
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

struct UVec {
	CYBOZU_ALIGN(64) Vec v[N];
	void put(const char *msg = nullptr) const
	{
		if (msg) printf("%s\n", msg);
		for (size_t i = 0; i < N; i++) {
			::put(v[i]);
		}
	}
};

static const CYBOZU_ALIGN(64) Unit _vmask[M] = {
	mask, mask, mask, mask, mask, mask, mask, mask
};

static CYBOZU_ALIGN(64) Unit _vrp[N];
static CYBOZU_ALIGN(64) Unit _vp[N*M];

static const Vec& vmask = *(const Vec*)_vmask;
//static const Vec& vone = *(const Vec*)_vone;
static const UVec& vp = *(const UVec*)_vp;
static const UVec& vrp = *(const UVec*)_vrp;

void cvt(UVec *_y, const Unit *x)
{
	Unit *y = (Unit *)_y;
	for (size_t i = 0; i < M; i++) {
		for (size_t j = 0; j < N; j++) {
			y[j*M+i] = x[i*N+j];
		}
	}
}

void cvt(Unit *y, const UVec *_x)
{
	const Unit *x = (const Unit *)_x;
	for (size_t j = 0; j < N; j++) {
		for (size_t i = 0; i < M; i++) {
			y[i*N+j] = x[j*M+i];
		}
	}
}

Vec vmulL(const Vec& a, const Vec& b, const Vec& c)
{
	return _mm512_madd52lo_epu64(a, b, c);
}

Vec vmulH(const Vec& a, const Vec& b, const Vec& c)
{
	return _mm512_madd52hi_epu64(a, b, c);
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

void vrawAdd(UVec& z, const UVec& x, const UVec& y)
{
	Vec t = vadd(x.v[0], y.v[0]);
	Vec c = vshl(t, W);
	z.v[0] = vand(t, vmask);

	for (size_t i = 1; i < N; i++) {
		t = vadd(x.v[i], y.v[i]);
		t = vadd(t, c);
		if (i == N-1) {
			z.v[i] = t;
			return;
		}
		c = vshl(t, W);
		z.v[i] = vand(t, vmask);
	}
}

Vmask vrawSub(UVec& z, const UVec& x, const UVec& y)
{
	Vec t = vsub(x.v[0], y.v[0]);
	Vec c = vshl(t, S);
	z.v[0] = vand(t, vmask);
	for (size_t i = 1; i < N; i++) {
		t = vsub(x.v[i], y.v[i]);
		t = vsub(t, c);
		c = vshl(t, S);
		z.v[i] = vand(t, vmask);
	}
	return vcmpneq(c, vzero());
}

void uvselect(UVec& z, const Vmask& c, const UVec& a, const UVec& b)
{
	for (size_t i = 0; i < N; i++) {
		z.v[i] = vselect(c, a.v[i], b.v[i]);
	}
}

void uvadd(UVec& z, const UVec& x, const UVec& y)
{
	UVec s, t;
	vrawAdd(s, x, y);
	Vmask c = vrawSub(t, s, vp);
	uvselect(z, c, s, t);
}

void uvsub(UVec& z, const UVec& x, const UVec& y)
{
	UVec s, t;
	Vmask c = vrawSub(s, x, y);
	vrawAdd(t, s, vp);
	t.v[N-1] = vand(t.v[N-1], vmask);
	uvselect(z, c, t, s);
}

// out = c ? a : b
void select(Unit *out, bool c, const Unit *a, const Unit *b)
{
	const Unit *o = c ? a : b;
	for (size_t i = 0; i < N; i++) {
		out[i] = o[i];
	}
}

template<size_t N>
void toArray(Unit x[N], mpz_class mx)
{
	for (size_t i = 0; i < N; i++) {
		mpz_class a = mx & mask;
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

template<size_t N>
void putArray(const Unit x[N], const char *msg = nullptr)
{
	if (msg) printf("%s ", msg);
	for (size_t i = 0; i < N; i++) {
		printf("%016lx ", x[N-1-i]);
	}
	printf("\n");
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
//	setArray(&b, z, &x, 1);
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
		return mcl::gmp::getUnit(x, 0) & mask;
	}
	void put() const
	{
		std::cout << std::hex;
		std::cout << "p=0x" << mp << std::endl;
		std::cout << "R=0x" << mR << std::endl;
		std::cout << "R2=0x" << mR2 << std::endl;
		std::cout << "rp=0x" << rp << std::endl;
	}
	explicit Montgomery(const mpz_class& _p)
	{
		mp = _p;
		mR = 1;
		mR = (mR << (W * N)) % mp;
		mR2 = (mR * mR) % mp;
		toArray<N>(v_, _p);
		putArray<N>(v_, "v_");
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
			Unit q = (getLow(z) * rp) & mask;
			mpz_class t = q;
			z += mp * t;
			z >>= W;
		}
		if (z >= mp) {
			z -= mp;
		}
	}
};

void rawAdd(Unit *z, const Unit *x, const Unit *y)
{
	Unit c = 0;
	for (size_t i = 0; i < N; i++) {
		z[i] = x[i] + y[i] + c;
		if (i == N-1) break;
		c = z[i] >> W;
		z[i] &= mask;
	}
}

bool rawSub(Unit *z, const Unit *x, const Unit *y)
{
	Unit c = 0;
	for (size_t i = 0; i < N; i++) {
		z[i] = x[i] - y[i] - c;
		c = z[i] >> S;
		z[i] &= mask;
	}
	return c != 0;
}

void add(Unit *z, const Unit *x, const Unit *y)
{
	Unit s[N], t[N];
	rawAdd(s, x, y);
	bool c = rawSub(t, s, p);
	select(z, c, s, t);
}

void sub(Unit *z, const Unit *x, const Unit *y)
{
	Unit s[N], t[N];
	bool c = rawSub(s, x, y);
	rawAdd(t, s, p);
	t[N-1] &= mask;
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
	return L & mask;
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
void mod(Unit *z, const Unit *xy, const Montgomery& mont)
{
	Unit t[N*2], q, H;
	for (size_t i = 0; i < N*2; i++) {
		t[i] = xy[i];
	}
	for (size_t i = 0; i < N; i++) {
		q = mul52bit(&H, t[i], mont.rp);
		t[N+i] += rawMulUnitAdd(t + i, mont.p, q);
		t[i+1] += t[i] >> W;
		t[i] &= mask;
	}
	for (size_t i = N; i < N*2-1; i++) {
		t[i+1] += t[i] >> W;
		t[i] &= mask;
	}
	bool c = rawSub(z, t + N, mont.p);
	select(z, c, t + N, z);
}

// z[N] = Montgomery mul(x[N], y[2N])
void mul(Unit *z, const Unit *x, const Unit *y, const Montgomery& mont)
{
	Unit t[N*2], q, H;
	rawMulUnit(t, x, y[0]);
	q = mul52bit(&H, t[0], mont.rp);
	t[N] += rawMulUnitAdd(t, mont.p, q);
	for (size_t i = 1; i < N; i++) {
		t[N+i] = rawMulUnitAdd(t+i, x, y[i]);
		t[i] += t[i-1] >> W;
		q = mul52bit(&H, t[i], mont.rp);
		t[N+i] += rawMulUnitAdd(t+i, mont.p, q);
	}
	for (size_t i = N; i < N*2; i++) {
		t[i] += t[i-1] >> W;
		t[i-1] &= mask;
	}
	bool c = rawSub(z, t+N, mont.p);
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
	return mx % mp;
}

void init(const Montgomery& mont)
{
	const char *pStr = "1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab";
	mp.set_str(pStr, 16);
	toArray<N>(p, mp);
	for (size_t i = 0; i < M; i++) {
		for (size_t j = 0; j < N; j++) {
			_vp[i*N+j] = p[i];
		}
		_vrp[i] = mont.rp;
	}
}

mpz_class madd(const mpz_class& x, const mpz_class& y)
{
	mpz_class z = x + y;
	if (z >= mp) z -= mp;
	return z;
}

mpz_class msub(const mpz_class& x, const mpz_class& y)
{
	mpz_class z = x - y;
	if (z < 0) z += mp;
	return z;
}

void putAll(const mpz_class& x, const mpz_class& y, const mpz_class& z, const mpz_class& w)
{
	std::cout << "x=" << x << std::endl;
	std::cout << "y=" << y << std::endl;
	std::cout << "z=" << z << std::endl;
	std::cout << "w=" << w << std::endl;
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
	CYBOZU_ALIGN(64) Unit _x[N*M], _y[N*M], _z[N*M];
	UVec x[M], y[M], z[M];
	for (size_t i = 0; i < M; i++) {
		mx[i] = (_mx + i * 123) % mp;
		my[i] = (_my + i * 245) % mp;
		toArray<N>(_x + i*N, mx[i]);
		toArray<N>(_y + i*N, my[i]);
	}

	cvt(x, _x);
	cvt(y, _y);

	// add
	for (size_t i = 0; i < M; i++) {
		uvadd(z[i], x[i], y[i]);
	}
	cvt(_z, z);

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
		uvsub(z[i], x[i], y[i]);
	}
	cvt(_z, z);

	for (size_t i = 0; i < M; i++) {
		mz = msub(mx[i], my[i]);
		mw = fromArray<N>(_z + i*N);
		if (mz != mw) {
			printf("uvsub err %zd\n", i);
			putAll(mx[i], my[i], mz, mw);
		}
	}
}

void testMont(const Montgomery& mont, const mpz_class& mx, const mpz_class& my)
{
	mpz_class ax, ay, axy, xy, mxy;
	ax = mont.toMont(mx);
	ay = mont.toMont(my);
	mont.mul(axy, ax, ay);
	xy = mont.fromMont(axy);
	mxy = (mx * my) % mp;
	if (xy != mxy) {
		puts("err mont");
		putAll(mx, my, mxy, xy);
	}
	Unit x[N], y[N], t[N*2], z[N];
	toArray<N>(x, ax);
	toArray<N>(y, ay);
	rawMul(t, x, y);
	mod(z, t, mont);
	mpz_class w = fromArray<N>(z);
	if (w != axy) {
		puts("err mont2");
		putAll(mx, my, axy, w);
	}
	memset(z, 0, sizeof(z));
	mul(z, x, y, mont);
	w = fromArray<N>(z);
	if (w != axy) {
		puts("err mont3");
		putAll(mx, my, axy, w);
	}
}

int main()
{
	Montgomery mont(mp);
	init(mont);
	mont.put();

	const mpz_class tbl[] = {
		0, 1, 2, mask-1, mask, mask+1, mp-1, mp>>2, 0x12345, mp-0x1111,
	};
	std::cout << std::hex;
	for (const auto& mx : tbl) {
		for (const auto& my : tbl) {
			test(mx, my);
			testMont(mont, mx, my);
			vtest(mx, my);
		}
	}

	cybozu::XorShift rg;
	for (int i = 0; i < 100; i++) {
		mpz_class mx = mpz_rand(rg);
		mpz_class my = mpz_rand(rg);
		test(mx, my);
		testMont(mont, mx, my);
		vtest(mx, my);
	}
	puts("ok");
}
