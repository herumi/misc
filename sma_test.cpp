#include "sma.hpp"
#include <cybozu/test.hpp>
#include <cybozu/inttype.hpp>
#include <numeric>

struct V {
	uint64_t v;
	uint64_t t;
};
void test(uint64_t interval, const V* tbl, size_t tblNum, uint64_t expectNum, uint64_t expectTotal)
{
	SMAverage sma(interval);
	for (size_t i = 0; i < tblNum; i++) {
		sma.append(tbl[i].v, tbl[i].t);
	}
	const SMAverage::ValVec& vv = sma.getValVec();
	CYBOZU_TEST_EQUAL(vv.size(), expectNum);
	CYBOZU_TEST_EQUAL(sma.getTotalVal(), expectTotal);
}

CYBOZU_TEST_AUTO(append)
{
	const V tbl[] = {
		{ 1, 0 },
		{ 2, 0 },
		{ 3, 0 },
		{ 4, 1 },
		{ 5, 1 },
		{ 6, 3 },
		{ 7, 5 },
		{ 8, 7 },
		{ 9, 9 },
	};
	test(10, tbl, 9, 9, 45); // all
	test(9, tbl, 9, 9, 45); // all
	test(8, tbl, 9, 6, 39); // tbl[3..8]
	test(7, tbl, 9, 4, 30); // tbl[5..8]
	test(6, tbl, 9, 4, 30); // tbl[5..8]
	test(5, tbl, 9, 3, 24); // tbl[6..8]
	test(4, tbl, 9, 3, 24); // tbl[6..8]
	test(3, tbl, 9, 2, 17); // tbl[7..8]
	test(2, tbl, 9, 2, 17); // tbl[7..8]
	test(1, tbl, 9, 1, 9); // tbl[8]
}
