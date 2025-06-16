#include "bbs.hpp"
#include <cybozu/test.hpp>
#include <set>

const size_t maxMsgSize = 32;

using namespace bbs;

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
	mclSize msgSize[N];
	const mclSize MSG_SIZE = 2;
	for (size_t i = 0; i < N; i++) msgSize[i] = MSG_SIZE;
	uint8_t msgs[N*MSG_SIZE];
	for (size_t i = 0; i < sizeof(msgs); i++) {
		msgs[i] = uint8_t(i);
	}

	for (size_t n = 1; n <= N; n++) {
		Signature sig;
		sig.sign(sec, pub, msgs, msgSize, n);
		CYBOZU_TEST_ASSERT(sig.verify(pub, msgs, msgSize, n));
		CYBOZU_TEST_ASSERT(!sig.verify(pub, msgs, msgSize, n - 1));
		msgs[0] -= 1;
		CYBOZU_TEST_ASSERT(!sig.verify(pub, msgs, msgSize, n));
	}
}

CYBOZU_TEST_AUTO(setJs)
{
	typedef std::set<mclSize> IntSet;

	const size_t msgN = 10;
	const struct {
		size_t discN;
		mclSize disc[msgN];
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
		mclSize js[msgN];
		for (size_t j = 0; j < msgN; j++) js[j] = mclSize(-1);
		const size_t discN = tbl[i].discN;
		const size_t undiscN = msgN  - discN;
		bbs::local::setJs(js, undiscN, tbl[i].disc, discN);
		IntSet is;
		for (size_t j = 0; j < discN; j++) is.insert(tbl[i].disc[j]);
		CYBOZU_TEST_EQUAL(is.size(), discN);
		for (size_t j = 0; j < undiscN; j++) is.insert(js[j]);
		CYBOZU_TEST_EQUAL(is.size(), msgN);
		for (size_t j = undiscN; j < msgN; j++) {
			CYBOZU_TEST_EQUAL(js[j], mclSize(-1));
		}
	}
}

void checkProof(const PublicKey& pub, const Signature& sig, const Fr *msgs, size_t msgN, const mclSize *discIdxs, size_t discN)
{
	const mclSize undiscN = msgN - discN;
	Fr *discMsgs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * discN);

	// select disclosed msgs
	for (size_t i = 0; i < discN; i++) discMsgs[i] = msgs[discIdxs[i]];

	Proof prf;
	Fr *m_hat = (Fr *)CYBOZU_ALLOCA(sizeof(Fr) * undiscN);
	prf.set(m_hat, undiscN);
	const uint8_t nonce[] = { 1, 2, 3, 4, 0x11, 0x22, 0x33 };
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msgs, msgN, discIdxs, discN, nonce, sizeof(nonce)));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, msgN, discMsgs, discIdxs, discN, nonce, sizeof(nonce)));
}

void checkProof(const PublicKey& pub, const Signature& sig, const uint8_t *msgs, const mclSize *msgSize, size_t msgN, const mclSize *discIdxs, size_t discN)
{
	const mclSize undiscN = msgN - discN;

	mclSize totalDiscMsgSize = 0;
	for (size_t i = 0; i < discN; i++) {
		totalDiscMsgSize += msgSize[discIdxs[i]];
	}
	uint8_t *discMsgs = (uint8_t*)CYBOZU_ALLOCA(totalDiscMsgSize);
	mclSize *discMsgSize = (mclSize*)CYBOZU_ALLOCA(sizeof(mclSize) * discN);
	// copy msgs to discMsgSize
	{
		const uint8_t *src = msgs;
		uint8_t *dst = discMsgs;
		size_t pos = 0;
		for (size_t i = 0; i < discN; i++) {
			// skip
			while (pos < discIdxs[i]) {
				src += msgSize[pos];
				pos++;
			}
			mclSize size = msgSize[pos];
			memcpy(dst, src, size);
			src += size;
			dst += size;
			discMsgSize[i] = size;
			pos++;
		}
	}

	Proof prf;
	Fr *m_hat = (Fr *)CYBOZU_ALLOCA(sizeof(Fr) * undiscN);
	prf.set(m_hat, undiscN);
	const uint8_t nonce[] = { 1, 2, 3, 4, 0x11, 0x22, 0x33 };
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msgs, msgSize, msgN, discIdxs, discN, nonce, sizeof(nonce)));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, msgN, discMsgs, discMsgSize, discIdxs, discN, nonce, sizeof(nonce)));
}

CYBOZU_TEST_AUTO(proof)
{
	const size_t msgN = 10;
	mclSize msgSize[msgN];
	mclSize totalMsgSize = 0;
	for (size_t i = 0; i < msgN; i++) {
		msgSize[i] = 1 + ((i*15+i) % 13);
		totalMsgSize += msgSize[i];
	}
	uint8_t *msgs = (uint8_t*)CYBOZU_ALLOCA(totalMsgSize);
	for (size_t i = 0; i < totalMsgSize; i++) {
		msgs[i] = uint8_t(i*i+123*i+21);
	}

	mclSize discIdxs[msgN];

	PublicKey pub;
	Signature sig;
	// setup public key and signature
	{
		SecretKey sec;
		sec.init();
		sec.getPublicKey(pub);
		sig.sign(sec, pub, msgs, msgSize, msgN);
	}

	puts("disclose nothing");
	checkProof(pub, sig, msgs, msgSize, msgN, 0, 0);

	puts("disclose one");
	{
		const size_t discN = 1;
		for (size_t i = 0; i < msgN; i++) {
			discIdxs[0] = i;
			checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
		}
	}
	puts("disclose two");
	{
		const size_t discN = 2;
		for (size_t i = 0; i < msgN; i++) {
			discIdxs[0] = i;
			for (size_t j = i + 1; j < msgN; j++) {
				discIdxs[1] = j;
				checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
			}
		}
	}
	puts("disclose three");
	{
		const size_t discN = 3;
		for (size_t i = 0; i < msgN; i++) {
			discIdxs[0] = i;
			for (size_t j = i + 1; j < msgN; j++) {
				discIdxs[1] = j;
				for (size_t k = j + 1; k < msgN; k++) {
					discIdxs[2] = k;
					checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
				}
			}
		}
	}
	puts("disclose all");
	{
		const size_t discN = msgN;
		for (size_t i = 0; i < discN; i++) discIdxs[i] = i;
		checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
	}
}
