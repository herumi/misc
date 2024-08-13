#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include "../../mcl/src/avx512.hpp"
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/test.hpp>

extern "C" {

void select0(Vec *z, const Vec *x, const Vec *y);
void select1(Vec *z, const Vec *x, const Vec *y);
void misc(Vec *z, const Vec *x, const Vec *y);

}

const size_t N = 8;
const uint64_t FF = uint64_t(-1);

const int C = 10000;

std::string toStr(const Vec& x)
{
	std::string s;
	char buf[64];
	const uint8_t *p = (const uint8_t*)&x;
	for (size_t i = 0; i < sizeof(x); i++) {
		snprintf(buf, sizeof(buf), "%02x", p[i]);
		s += buf;
		if ((i & 7) == 7) s += ' ';
	}
	return s;
}


void dump(const Vec& x, const char *msg = nullptr)
{
	if (msg) {
		printf("%s\n", msg);
	}
	printf("%s\n", toStr(x).c_str());
}

void set(Vec& x, const uint64_t a[N])
{
	memcpy(&x, a, sizeof(x));
}

void get(uint64_t a[N], const Vec& x)
{
	memcpy(a, &x, sizeof(x));
}

void set(Vec& x, cybozu::XorShift& rg)
{
	uint64_t a[N];
	for (size_t i = 0; i < N; i++) {
		a[i] = rg.get64();
	}
	set(x, a);
}

bool isEqual(const Vec& x, const Vec& y)
{
	return memcmp(&x, &y, sizeof(x)) == 0;
}

std::ostream& operator<<(std::ostream& os, const Vec& x)
{
	return os << toStr(x);
}

CYBOZU_TEST_AUTO(select)
{
	Vec z[N], w[N], x[N], y[N];
	cybozu::XorShift rg;
	for (size_t i = 0; i < N; i++) {
		set(x[i], rg);
		set(y[i], rg);
	}
	select0(z, x, y);
	select0(w, x, y);
	for (size_t i = 0; i < N; i++) {
		CYBOZU_TEST_ASSERT(isEqual(z[i], w[i]));
	}
	CYBOZU_BENCH_C("select0", C, select0, z, z, x);
	CYBOZU_BENCH_C("select1", C, select1, z, z, x);
}

CYBOZU_TEST_AUTO(misc)
{
	Vec z, x, y;
	uint64_t ax[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	uint64_t ay[] = { 0, 0, 0, 0, 0, 5, 0, 0, 0 };
	uint64_t az[N];
	uint64_t aw[] = { FF, 0, 0, 0, 0, FF, 0, 0, 0 };
	set(x, ax);
	set(y, ay);
	misc(&z, &x, &y);
	get(az, z);
	CYBOZU_TEST_EQUAL_ARRAY(az, aw, N);
}
