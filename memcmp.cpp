#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/endian.hpp>

int mysql_memcmp(const uint8_t *data1, const uint8_t *data2, size_t len)
{
#if 1
	if (len) {
		for (size_t i = 4 + (len & 3); i > 0; i--) {
			int cmp = int(*data1++) - int(*data2++);
			if (cmp) {
				return cmp;
			}
			if (!--len) {
				break;
			}
		}
	}
#endif
	if (len) {
		int cmp = memcmp(data1, data2, len);
		if (cmp) {
			return cmp;
		}
	}
	return 0;
}

int mie_memcmp(const uint8_t *data1, const uint8_t *data2, size_t len)
{
	if (len == 0) return 0;
#if 0
	if (len >= 4) {
		uint32_t a = cybozu::Get32bitAsBE(data1);
		uint32_t b = cybozu::Get32bitAsBE(data2);
		if (a > b) return 1;
		if (a < b) return -1;
		data1 += 4;
		data2 += 4;
		len -= 4;
	}
	return memcmp(data1, data2, len);
#else
	int c;
	switch (len) {
	default:
	case 2: c = int(*data1++) - int(*data2++); len--; if (c) return c;
	case 1: c = int(*data1++) - int(*data2++); len--; if (c) return c;
	}
	if (len) {
		int cmp = memcmp(data1, data2, len);
		if (cmp) {
			return cmp;
		}
	}
	return 0;
#endif
}

int normalize(int a)
{
	return a < 0 ? -1 : a > 0 ? 1 : 0;
}


CYBOZU_TEST_AUTO(memcmp1)
{
	const uint8_t ptn[] = { 0, 1, 2, 0x7f, 0x80, 0xff };
	const size_t n = sizeof(ptn) / sizeof(ptn[0]);
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			const uint8_t *p = &ptn[i];
			const uint8_t *q = &ptn[j];
			int a = normalize(memcmp(p, q, 1));
			int b = normalize(mie_memcmp(p, q, 1));
			CYBOZU_TEST_EQUAL(a, b);
		}
	}
}

template<size_t N>
void test()
{
	cybozu::XorShift rg;
	for (size_t i = 0; i < 100; i++) {
		uint8_t p[N];
		uint8_t q[N];
		rg.read(p, N);
		rg.read(q, N);
		int a = normalize(memcmp(p, q, N));
		int b = normalize(mie_memcmp(p, q, N));
		CYBOZU_TEST_EQUAL(a, b);
	}
}

CYBOZU_TEST_AUTO(memcmpN)
{
	test<1>();
	test<2>();
	test<3>();
	test<4>();
	test<5>();
}

template<class F>
void bench(const char *msg, F f)
{
	cybozu::XorShift rg;
	const size_t size = 64;
	std::vector<uint8_t> buf(size * 2);
	rg.read(&buf[0], buf.size());

	cybozu::CpuClock clk;
	int ret = 0;
	const size_t N = 1000000;
	for (size_t i = 0; i < N; i++) {
		size_t p = rg.get32() % size;
		size_t q = rg.get32() % size;
		size_t n = rg.get32() % size;
		clk.begin();
		int a = normalize(f(&buf[p], &buf[q], n));
		clk.end();
		ret += a;
	}
	printf("%s ", msg);
	clk.put();
	printf("%d\n", ret);
}

CYBOZU_TEST_AUTO(bench)
{
	bench("memcmp", memcmp);
	bench("mie_memcmp", mie_memcmp);
	bench("mysql_memcmp", mysql_memcmp);
}
