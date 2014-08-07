#include "veb_tree.hpp"
#include <set>
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/unordered_set.hpp>
#include <cybozu/xorshift.hpp>

CYBOZU_TEST_AUTO(test1)
{
	const uint32_t M = 1 << 16;
	VebTree T(M, 0);
	T.insert(0x000f);
	CYBOZU_TEST_EQUAL(T.findNext(0x000f), 0x000fu);
	T.insert(0x00f0);
	CYBOZU_TEST_EQUAL(T.findNext(0x00f0), 0x00f0u);
	T.insert(0x0f00);
	CYBOZU_TEST_EQUAL(T.findNext(0x0f00), 0x0f00u);
	T.insert(0xf000);
	CYBOZU_TEST_EQUAL(T.findNext(0xf000), 0xf000u);
	uint32_t x = T.findNext(0);
	CYBOZU_TEST_EQUAL(x, 0x000fu);
	x = T.findNext(x + 1);
	CYBOZU_TEST_EQUAL(x, 0x00f0u);
	x = T.findNext(x + 1);
	CYBOZU_TEST_EQUAL(x, 0x0f00u);
	x = T.findNext(x + 1);
	CYBOZU_TEST_EQUAL(x, 0xf000u);
	x = T.findNext(x + 1);
	CYBOZU_TEST_EQUAL(x, M);
}

uint32_t fill_test2(VebTree& T, uint32_t m)
{
	uint32_t n = 0;
	for (uint32_t i = 0; i < m; i++) {
		uint32_t x = rand() % T.getM();
		uint32_t v = T.findNext(x);
		if (v != x) {
			T.insert(x);
			n++;
		}
	}
	return n;
}

CYBOZU_TEST_AUTO(test2)
{
	srand(83843);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, 0);
	uint32_t m = fill_test2(T, 1000);
	uint32_t n = 0;
	uint32_t i = T.findNext(0);
	while (i != M) {
		n++;
		i = T.findNext(i + 1);
	}
	CYBOZU_TEST_EQUAL(n, m);
}

void fill_test3(VebTree& T, uint32_t M)
{
	for (uint32_t i = 0; i < 0xff; i++) {
		uint32_t x = rand() % M;
		T.insert(x);
		CYBOZU_TEST_EQUAL(T.findNext(x), x);
	}
}

CYBOZU_TEST_AUTO(test3)
{
	srand(438749);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, 0);
	fill_test3(T, M);
	uint32_t i = T.findNext(0);
	while (i < M) {
		T.erase(i);
		uint32_t j = T.findNext(i);
		CYBOZU_TEST_ASSERT(i != j);
		i = j;
	}
}

CYBOZU_TEST_AUTO(test4)
{
	const uint32_t M = 1 << 16;
	VebTree T(M, 0);
	T.insert(0x000f);
	CYBOZU_TEST_ASSERT(T.findNext(0x000f) == 0x000f);
	T.insert(0x00f0);
	CYBOZU_TEST_ASSERT(T.findNext(0x00f0) == 0x00f0);
	T.insert(0x0f00);
	CYBOZU_TEST_ASSERT(T.findNext(0x0f00) == 0x0f00);
	T.insert(0xf000);
	CYBOZU_TEST_ASSERT(T.findNext(0xf000) == 0xf000);
	T.erase(0x000f);
	CYBOZU_TEST_ASSERT(T.findNext(0x000f) != 0x000f);
	T.erase(0x00f0);
	CYBOZU_TEST_ASSERT(T.findNext(0x00f0) != 0x00f0);
	T.erase(0x0f00);
	CYBOZU_TEST_ASSERT(T.findNext(0x0f00) != 0x0f00);
	T.erase(0xf000);
	CYBOZU_TEST_ASSERT(T.findNext(0xf000) != 0xf000);
}

CYBOZU_TEST_AUTO(test5)
{
	const uint32_t M = 1 << 16;
	VebTree T(M, 0);
	T.insert(0xf000);
	CYBOZU_TEST_ASSERT(T.findPrev(0xf000) == 0xf000);
	T.insert(0x0f00);
	CYBOZU_TEST_ASSERT(T.findPrev(0x0f00) == 0x0f00);
	T.insert(0x00f0);
	CYBOZU_TEST_ASSERT(T.findPrev(0x00f0) == 0x00f0);
	T.insert(0x000f);
	CYBOZU_TEST_ASSERT(T.findPrev(0x000f) == 0x000f);
	uint32_t x = T.findPrev(M - 1);
	CYBOZU_TEST_ASSERT(x == 0xf000);
	x = T.findPrev(x - 1);
	CYBOZU_TEST_ASSERT(x == 0x0f00);
	x = T.findPrev(x - 1);
	CYBOZU_TEST_ASSERT(x == 0x00f0);
	x = T.findPrev(x - 1);
	CYBOZU_TEST_ASSERT(x == 0x000f);
	x = T.findPrev(x - 1);
	CYBOZU_TEST_ASSERT(x == M);
}

uint32_t fill_test6(VebTree& T, uint32_t m)
{
	uint32_t n = 0;
	for (uint32_t i = 0; i < m; i++) {
		uint32_t x = rand() % T.getM();
		if (T.findPrev(x) != x) {
			T.insert(x);
			n++;
		}
	}
	return n;
}

CYBOZU_TEST_AUTO(test6)
{
	srand(83843);
	uint32_t M = rand() % (1 << 16);
	VebTree T(M, 0);
	uint32_t m = fill_test6(T, 1000);
	uint32_t n = 0;
	uint32_t i = T.findPrev(M - 1);
	while (i != M) {
		n++;
		i = T.findPrev(i - 1);
	}
	CYBOZU_TEST_ASSERT(n == m);
}

void fill_test7(VebTree& T, uint32_t M)
{
	for (uint32_t i = 0; i < 1000; i++) {
		uint32_t x = rand() % M;
		T.insert(x);
	}
}

CYBOZU_TEST_AUTO(test7)
{
	srand(433849);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, 0);
	fill_test7(T, M);
	uint32_t i = T.findPrev(M - 1);
	while (i < M) {
		T.erase(i);
		uint32_t j = T.findPrev(i);
		CYBOZU_TEST_ASSERT(i != j);
		i = j;
	}
}

CYBOZU_TEST_AUTO(test8)
{
	const uint32_t M = 1 << 16;
	VebTree T(M, 0);
	T.insert(0xf000);
	CYBOZU_TEST_ASSERT(T.findPrev(0xf000) == 0xf000);
	T.insert(0x0f00);
	CYBOZU_TEST_ASSERT(T.findPrev(0x0f00) == 0x0f00);
	T.insert(0x00f0);
	CYBOZU_TEST_ASSERT(T.findPrev(0x00f0) == 0x00f0);
	T.insert(0x000f);
	CYBOZU_TEST_ASSERT(T.findPrev(0x000f) == 0x000f);
	T.erase(0xf000);
	CYBOZU_TEST_ASSERT(T.findPrev(0xf000) != 0xf000);
	T.erase(0x0f00);
	CYBOZU_TEST_ASSERT(T.findPrev(0x0f00) != 0x0f00);
	T.erase(0x00f0);
	CYBOZU_TEST_ASSERT(T.findPrev(0x00f0) != 0x00f0);
	T.erase(0x000f);
	CYBOZU_TEST_ASSERT(T.findPrev(0x000f) != 0x000f);
}

uint32_t reduce_test9(VebTree& T, uint32_t m)
{
	uint32_t n = 0;
	for (uint32_t i = 0; i < m; i++) {
		uint32_t x = rand() % T.getM();
		if (T.findNext(x) == x) {
			T.erase(x);
			n++;
		}
	}
	return n;
}

CYBOZU_TEST_AUTO(test9)
{
	srand(83843);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, 1);
	uint32_t m = reduce_test9(T, 1000);
	uint32_t n = 0;
	uint32_t i = T.findNext(0);
	while (i != M) {
		n++;
		i = T.findNext(i + 1);
	}
	CYBOZU_TEST_ASSERT(n == M - m);
}

void fill_test10(VebTree& T, uint32_t M)
{
	for (int i = 0; i < 0xff; i++) {
		uint32_t x = rand() % M;
		T.erase(x);
		CYBOZU_TEST_ASSERT(T.findNext(x) != x);
	}
}

CYBOZU_TEST_AUTO(test10)
{
	srand(438749);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, true);
	fill_test10(T, M);
	uint32_t i = T.findNext(0);
	while (i < M) {
		T.erase(i);
		uint32_t j = T.findNext(i);
		CYBOZU_TEST_ASSERT(i != j);
		i = j;
	}
}

CYBOZU_TEST_AUTO(test11)
{
	uint32_t M = 1 << 16;
	VebTree T(M, 1);
	T.erase(0x000f);
	CYBOZU_TEST_ASSERT(T.findNext(0x000f) != 0x000f);
	T.erase(0x00f0);
	CYBOZU_TEST_ASSERT(T.findNext(0x00f0) != 0x00f0);
	T.erase(0x0f00);
	CYBOZU_TEST_ASSERT(T.findNext(0x0f00) != 0x0f00);
	T.erase(0xf000);
	CYBOZU_TEST_ASSERT(T.findNext(0xf000) != 0xf000);
	T.insert(0x000f);
	CYBOZU_TEST_ASSERT(T.findNext(0x000f) == 0x000f);
	T.insert(0x00f0);
	CYBOZU_TEST_ASSERT(T.findNext(0x00f0) == 0x00f0);
	T.insert(0x0f00);
	CYBOZU_TEST_ASSERT(T.findNext(0x0f00) == 0x0f00);
	T.insert(0xf000);
	CYBOZU_TEST_ASSERT(T.findNext(0xf000) == 0xf000);
}

uint32_t reduce_test12(VebTree& T, uint32_t m)
{
	uint32_t n = 0;
	for (uint32_t i = 0; i < m; i++) {
		uint32_t x = rand() % T.getM();
		if (T.findPrev(x) == x) {
			T.erase(x);
			n++;
		}
	}
	return n;
}

CYBOZU_TEST_AUTO(test12)
{
	srand(83843);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, 1);
	uint32_t m = reduce_test12(T, 1000);
	uint32_t n = 0;
	uint32_t i = T.findPrev(M - 1);
	while (i != M) {
		n++;
		i = T.findPrev(i - 1);
	}
	CYBOZU_TEST_EQUAL(n, M - m);
}

void fill_test13(VebTree& T, uint32_t M)
{
	for (int i = 0; i < 0xff; i++) {
		uint32_t x = rand() % M;
		T.erase(x);
		CYBOZU_TEST_ASSERT(T.findPrev(x) != x);
	}
}

CYBOZU_TEST_AUTO(test13)
{
	srand(438749);
	const uint32_t M = rand() % (1 << 16);
	VebTree T(M, true);
	fill_test13(T, M);
	uint32_t i = T.findPrev(M - 1);
	while (i < M) {
		T.erase(i);
		uint32_t j = T.findPrev(i);
		CYBOZU_TEST_ASSERT(i != j);
		i = j;
	}
}

CYBOZU_TEST_AUTO(test14)
{
	const uint32_t M = 1 << 16;
	VebTree T(M, true);
	T.erase(0xf000);
	CYBOZU_TEST_ASSERT(T.findPrev(0xf000) != 0xf000);
	T.erase(0x0f00);
	CYBOZU_TEST_ASSERT(T.findPrev(0x0f00) != 0x0f00);
	T.erase(0x00f0);
	CYBOZU_TEST_ASSERT(T.findPrev(0x00f0) != 0x00f0);
	T.erase(0x000f);
	CYBOZU_TEST_ASSERT(T.findPrev(0x000f) != 0x000f);
	T.insert(0xf000);
	CYBOZU_TEST_ASSERT(T.findPrev(0xf000) == 0xf000);
	T.insert(0x0f00);
	CYBOZU_TEST_ASSERT(T.findPrev(0x0f00) == 0x0f00);
	T.insert(0x00f0);
	CYBOZU_TEST_ASSERT(T.findPrev(0x00f0) == 0x00f0);
	T.insert(0x000f);
	CYBOZU_TEST_ASSERT(T.findPrev(0x000f) == 0x000f);
}

template<class Set>
void bench(const char *msg, uint32_t n, uint32_t r)
{
	printf("bench:%s -------------------\n", msg);
	Set set(n);
	cybozu::XorShift rg;
	for (uint32_t i = 0; i < n / r; i++) {
		set.insert(rg.get32() % n);
	}
	int c = 0;
	CYBOZU_BENCH_C("has", int(n / r), c += set.has, rg.get32() % n);
	printf("c=%d\n", c);
}

struct IntSet : std::set<uint32_t> {
	explicit IntSet(uint32_t)
	{
	}
	bool has(uint32_t x) const
	{
		return this->find(x) != this->end();
	}
};
struct IntHash : CYBOZU_NAMESPACE_STD::unordered_set<uint32_t> {
	explicit IntHash(uint32_t)
	{
	}
	bool has(uint32_t x) const
	{
		return this->find(x) != this->end();
	}
};

int main(int argc, char *argv[])
	try
{
	if (argc == 1) {
		return cybozu::test::autoRun.run(argc, argv);
	}
	const uint32_t n = atoi(argv[1]);
	const uint32_t r = argc > 2 ? atoi(argv[2]) : 2;
	printf("n=%u, r=%u\n", n, r);
	bench<IntSet>("std::set", n, r);
	bench<IntHash>("std::unordered_set", n, r);
	bench<VebTree>("vEB tree", n, r);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
