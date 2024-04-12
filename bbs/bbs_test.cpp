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
//	sec.initForDebug(123);
	sec.init();
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

void checkProof(const PublicKey& pub, const Signature& sig, const Fr *msgs, size_t L, const uint32_t *discIdxs, size_t R)
{
	const uint32_t U = L - R;
	Fr *discMsgs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * R);

	// select disclosed msgs
	for (size_t i = 0; i < R; i++) discMsgs[i] = msgs[discIdxs[i]];

	Proof prf;
	Fr *m_hat = (Fr *)CYBOZU_ALLOCA(sizeof(Fr) * U);
	prf.set(m_hat, U);
	const uint8_t nonce[] = { 1, 2, 3, 4, 0x11, 0x22, 0x33 };
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msgs, L, discIdxs, R, nonce, sizeof(nonce)));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, L, discMsgs, discIdxs, R, nonce, sizeof(nonce)));
}

CYBOZU_TEST_AUTO(proof)
{
	const size_t L = 10;
	Fr msgs[L];
	uint32_t discIdxs[L];
	{
		int v = 123;
		for (size_t i = 0; i < L; i++) msgs[i] = v++;
	}
	PublicKey pub;
	Signature sig;
	// setup public key and signature
	{
		SecretKey sec;
		sec.init();
		sec.getPublicKey(pub);
		sig.sign(sec, pub, msgs, L);
	}

	puts("disclose nothing");
	checkProof(pub, sig, msgs, L, 0, 0);

	puts("disclose one");
	{
		const size_t R = 1;
		for (size_t i = 0; i < L; i++) {
			discIdxs[0] = i;
			checkProof(pub, sig, msgs, L, discIdxs, R);
		}
	}
	puts("disclose two");
	{
		const size_t R = 2;
		for (size_t i = 0; i < L; i++) {
			discIdxs[0] = i;
			for (size_t j = i + 1; j < L; j++) {
				discIdxs[1] = j;
				checkProof(pub, sig, msgs, L, discIdxs, R);
			}
		}
	}
	puts("disclose three");
	{
		const size_t R = 3;
		for (size_t i = 0; i < L; i++) {
			discIdxs[0] = i;
			for (size_t j = i + 1; j < L; j++) {
				discIdxs[1] = j;
				for (size_t k = j + 1; k < L; k++) {
					discIdxs[2] = k;
					checkProof(pub, sig, msgs, L, discIdxs, R);
				}
			}
		}
	}
	puts("disclose all");
	{
		const size_t R = L;
		for (size_t i = 0; i < R; i++) discIdxs[i] = i;
		checkProof(pub, sig, msgs, L, discIdxs, R);
	}
}
