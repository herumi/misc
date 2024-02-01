#include <stdint.h>
#include <stdio.h>
#include <gmpxx.h>
#include <iostream>
#include <cybozu/xorshift.hpp>

typedef uint64_t Unit;
const size_t S = 52;
const size_t N = 8;
const uint64_t mask = (Unit(1) << S) - 1;

mpz_class mp;

// split into 52 bits
Unit p[N];

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
		mx >>= S;
	}
}

template<size_t N>
mpz_class fromArray(const Unit x[N])
{
	mpz_class mx = x[N-1];
	for (size_t i = 1; i < N; i++) {
		mx <<= S;
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
		mR = (mR << (S * N)) % mp;
		mR2 = (mR * mR) % mp;
		toArray<N>(v_, _p);
		putArray<N>(v_, "v_");
		rp = mcl::bint::getMontgomeryCoeff(v_[0], S);
		printf("rp=%zx\n", rp);
		p = v_;
		isFullBit = p[N - 1] >> (S - 1);
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
			z >>= S;
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
		c = z[i] >> S;
		z[i] &= mask;
	}
}

bool rawSub(Unit *z, const Unit *x, const Unit *y)
{
	Unit c = 0;
	for (size_t i = 0; i < N; i++) {
		z[i] = x[i] - y[i] - c;
		c = z[i] >> 63;
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

// z[N+1] += x[N] * y
void rawMulUnitAdd(Unit *z, const Unit *x, Unit y)
{
	Unit L[N], H[N];
	for (size_t i = 0; i < N; i++) {
		L[i] = mul52bit(&H[i], x[i], y);
	}
	z[0] += L[0];
	for (size_t i = 1; i < N; i++) {
		z[i] += L[i] + H[i-1];
	}
	z[N] = H[N-1];
}

void rawMul(Unit *z, const Unit *x, const Unit *y)
{
	rawMulUnit(z, x, y[0]);
	for (size_t i = 1; i < N; i++) {
		rawMulUnitAdd(z + i, x, y[i]);
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
	return mx % mp;
}

void init()
{
	const char *pStr = "1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab";
	mp.set_str(pStr, 16);
	toArray<N>(p, mp);
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
}

int main()
{
	init();
	Montgomery mont(mp);
	mont.put();

	const mpz_class tbl[] = {
		0, 1, 2, mask-1, mask, mask+1, mp-1, mp>>2, 0x12345, mp-0x1111,
	};
	std::cout << std::hex;
	for (const auto& mx : tbl) {
		for (const auto& my : tbl) {
			test(mx, my);
			testMont(mont, mx, my);
		}
	}

	cybozu::XorShift rg;
	for (int i = 0; i < 100; i++) {
		mpz_class mx = mpz_rand(rg);
		mpz_class my = mpz_rand(rg);
		test(mx, my);
	}
}
