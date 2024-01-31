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

void add(Unit *z, const Unit *x, const Unit *y)
{
	Unit t[N];
	Unit c = 0;
	for (size_t i = 0; i < N; i++) {
		t[i] = x[i] + y[i] + c;
		if (i == N-1) break;
		c = t[i] >> S;
		t[i] &= mask;
	}
	Unit s[N];
	c = 0;
	for (size_t i = 0; i < N; i++) {
		s[i] = t[i] - p[i] - c;
		c = s[i] >> 63;
		s[i] &= mask;
	}
	const Unit *o = c ? t : s;
	for (size_t i = 0; i < N; i++) {
		z[i] = o[i];
	}
}

void toArray(Unit x[N], mpz_class mx)
{
	for (size_t i = 0; i < N; i++) {
		mpz_class a = mx & mask;
		x[i] = a.get_ui();
		mx >>= S;
	}
}

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
	toArray(p, mp);
}

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
			mpz_class mz = madd(mx, my);
			Unit x[N], y[N], z[N];
			toArray(x, mx);
			toArray(y, my);
			add(z, x, y);
/*
			putArray(x, "x");
			putArray(y, "y");
			putArray(z, "z");
*/
			mpz_class e = fromArray(z);
			if (mz != e) {
				printf("err\n");
				std::cout << "mx=" << mx << std::endl;
				std::cout << "my=" << my << std::endl;
				std::cout << "mz=" << mz << std::endl;
				std::cout << "e =" << e << std::endl;
			}
		}
	}
}
