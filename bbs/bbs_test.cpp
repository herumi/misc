#include "bbs.hpp"
#include <cybozu/test.hpp>
#include <set>

const size_t maxMsgSize = 32;

using namespace mcl::bbs;

CYBOZU_TEST_AUTO(init)
{
	init(maxMsgSize);
}

CYBOZU_TEST_AUTO(sign_verify)
{
	SecretKey sec;
	sec.initForDebug(123);
	PublicKey pub;
	sec.getPublicKey(pub);
	Fr msg;
	msg = 0x12345678;
	Signature sig;
	sig.sign(sec, pub, &msg, 1);
	CYBOZU_TEST_ASSERT(sig.verify(pub, &msg, 1));
	msg -= 1;
	CYBOZU_TEST_ASSERT(!sig.verify(pub, &msg, 1));
}

CYBOZU_TEST_AUTO(setJs)
{
	typedef std::set<uint32_t> IntSet;

	const size_t L = 10;
	const struct {
		size_t R;
		uint32_t disc[L];
	} tbl[] = {
		{ 0, {} },
		{ 10, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
		{ 1, { 0 } },
		{ 1, { 5 } },
		{ 1, { 9 } },
		{ 2, { 0, 1 } },
		{ 2, { 3, 7 } },
		{ 2, { 3, 9 } },
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		uint32_t js[L];
		for (size_t j = 0; j < L; j++) js[j] = uint32_t(-1);
		const size_t R = tbl[i].R;
puts("disc");
for (size_t j = 0; j < R; j++) printf("%d ", tbl[i].disc[j]);
printf("\n");
		const size_t U = L  - R;
		mcl::bbs::local::setJs(js, L, tbl[i].disc, R);
puts("js");
for (size_t j = 0; j < U; j++) printf("%d ", js[j]);
printf("\n");
		IntSet is;
		for (size_t j = 0; j < R; j++) is.insert(tbl[i].disc[j]);
		CYBOZU_TEST_EQUAL(is.size(), R);
		for (size_t j = 0; j < U; j++) is.insert(js[j]);
		CYBOZU_TEST_EQUAL(is.size(), L);
		for (size_t j = U; j < L; j++) {
			CYBOZU_TEST_EQUAL(js[j], uint32_t(-1));
		}
	}
}

