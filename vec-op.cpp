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

void init()
{
	const char *pStr = "1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab";
	mp.set_str(pStr, 16);
	toArray<N>(p, mp);
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
		printf("err add\n");
		putAll(mx, my, mz, mw);
	}

	mz = msub(mx, my);
	sub(z, x, y);
	mw = fromArray<N>(z);
	if (mz != mw) {
		printf("err sub\n");
		putAll(mx, my, mz, mw);
	}
}

int main()
{
	cybozu::XorShift rg;
	init();

	const mpz_class tbl[] = {
		0, 1, 2, mp-1, mp>>2, 0x12345, mp-0x1111,
	};
	std::cout << std::hex;
	for (const auto& mx : tbl) {
		for (const auto& my : tbl) {
			test(mx, my);
		}
	}
}
