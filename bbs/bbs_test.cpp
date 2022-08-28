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
	const size_t N = 10;
	Fr msg[N];
	int v = 123;
	for (size_t i = 0; i < N; i++) msg[i] = v++;

	for (size_t n = 1; n <= N; n++) {
		Signature sig;
		sig.sign(sec, pub, msg, n);
		CYBOZU_TEST_ASSERT(sig.verify(pub, msg, n));
		CYBOZU_TEST_ASSERT(!sig.verify(pub, msg, n - 1));
		msg[0] -= 1;
		CYBOZU_TEST_ASSERT(!sig.verify(pub, msg, n));
	}
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
		{ 3, { 0, 1, 5 } },
		{ 3, { 0, 1, 9 } },
		{ 4, { 0, 2, 4, 8 } },
		{ 4, { 3, 4, 5, 6 } },
		{ 5, { 1, 3, 5, 7, 9 } },
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		uint32_t js[L];
		for (size_t j = 0; j < L; j++) js[j] = uint32_t(-1);
		const size_t R = tbl[i].R;
		const size_t U = L  - R;
		mcl::bbs::local::setJs(js, U, tbl[i].disc, R);
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

CYBOZU_TEST_AUTO(proof)
{
	SecretKey sec;
	sec.initForDebug(123);
	PublicKey pub;
	sec.getPublicKey(pub);
	const size_t L = 10;
	Fr msg[L];
	uint32_t discIdxs[L];
	int v = 123;
	for (size_t i = 0; i < L; i++) msg[i] = v++;
	Signature sig;
	sig.sign(sec, pub, msg, L);

	Fr disc;
	Proof prf;
	Fr m_hat[L];

	puts("disclose all");
	for (uint32_t i = 0; i < L; i++) discIdxs[i] = i;
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msg, L, discIdxs, L));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, L, msg, discIdxs, L));

	puts("disclose nothing");
	prf.set(m_hat, L);
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msg, L, 0, 0));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, L, msg, 0, 0));

	puts("disclose one");
	const uint32_t U = 1;
	const size_t R = L - U;
	prf.set(m_hat, R);
	uint32_t discIdx;
	for (size_t i = 0; i < L; i++) {
		discIdx = i;
		CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msg, L, &discIdx, 1));
		Fr discMsg = msg[i];
		CYBOZU_TEST_ASSERT(proofVerify(pub, prf, L, &discMsg, &discIdx, 1));
	}
}
