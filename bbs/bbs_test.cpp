#include "bbs.hpp"
#include <cybozu/test.hpp>
#include <set>

const size_t maxMsgSize = 32;

using namespace bbs;
using namespace mcl;

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
	bbsSecretKey csec;
	bbsInitSecretKey(&csec);
	bbsPublicKey cpub;
	bbsGetPublicKey(&cpub, &csec);
	const uint32_t N = 10;
	uint32_t msgSize[N];
	const uint32_t MSG_SIZE = 2;
	for (size_t i = 0; i < N; i++) msgSize[i] = MSG_SIZE;
	uint8_t msgs[N*MSG_SIZE];
	for (size_t i = 0; i < sizeof(msgs); i++) {
		msgs[i] = uint8_t(i);
	}

	for (uint32_t n = 1; n <= N; n++) {
		Signature sig;
		sig.sign(sec, pub, msgs, msgSize, n);
		CYBOZU_TEST_ASSERT(sig.verify(pub, msgs, msgSize, n));
		CYBOZU_TEST_ASSERT(!sig.verify(pub, msgs, msgSize, n - 1));
		msgs[0] -= 1;
		CYBOZU_TEST_ASSERT(!sig.verify(pub, msgs, msgSize, n));

		bbsSignature csig;
		bbsSign(&csig, &csec, &cpub, msgs, msgSize, n);
		CYBOZU_TEST_ASSERT(bbsVerify(&csig, &cpub, msgs, msgSize, n));
		CYBOZU_TEST_ASSERT(!bbsVerify(&csig, &cpub, msgs, msgSize, n - 1));
	}
}

CYBOZU_TEST_AUTO(setJs)
{
	typedef std::set<uint32_t> IntSet;

	const size_t msgN = 10;
	const struct {
		uint32_t discN;
		uint32_t disc[msgN];
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
		uint32_t js[msgN];
		for (size_t j = 0; j < msgN; j++) js[j] = uint32_t(-1);
		const uint32_t discN = tbl[i].discN;
		const uint32_t undiscN = msgN  - discN;
		bbs::local::setJs(js, undiscN, tbl[i].disc, discN);
		IntSet is;
		for (size_t j = 0; j < discN; j++) is.insert(tbl[i].disc[j]);
		CYBOZU_TEST_EQUAL(is.size(), discN);
		for (size_t j = 0; j < undiscN; j++) is.insert(js[j]);
		CYBOZU_TEST_EQUAL(is.size(), msgN);
		for (size_t j = undiscN; j < msgN; j++) {
			CYBOZU_TEST_EQUAL(js[j], uint32_t(-1));
		}
	}
}

void copyMsgsToDisc(uint8_t *discMsgs, uint32_t *discMsgSize, const uint8_t *msgs, const uint32_t *msgSize, const uint32_t *discIdxs, uint32_t discN)
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
		uint32_t size = msgSize[pos];
		memcpy(dst, src, size);
		src += size;
		dst += size;
		discMsgSize[i] = size;
		pos++;
	}
}

void checkProof(const PublicKey& pub, const Signature& sig, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN, const uint32_t *discIdxs, uint32_t discN)
{
	const uint32_t undiscN = msgN - discN;

	uint32_t totalDiscMsgSize = 0;
	for (size_t i = 0; i < discN; i++) {
		totalDiscMsgSize += msgSize[discIdxs[i]];
	}
	uint8_t *discMsgs = (uint8_t*)CYBOZU_ALLOCA(totalDiscMsgSize);
	uint32_t *discMsgSize = (uint32_t*)CYBOZU_ALLOCA(sizeof(uint32_t) * discN);
	// copy msgs to discMsgSize
	copyMsgsToDisc(discMsgs, discMsgSize, msgs, msgSize, discIdxs, discN);

	Proof prf;
	Fr *m_hat = (Fr *)CYBOZU_ALLOCA(sizeof(Fr) * undiscN);
	prf.set(m_hat, undiscN);
	const uint8_t nonce[] = { 1, 2, 3, 4, 0x11, 0x22, 0x33 };
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msgs, msgSize, msgN, discIdxs, discN, nonce, sizeof(nonce)));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, msgN, discMsgs, discMsgSize, discIdxs, discN, nonce, sizeof(nonce)));
}

void ccheckProof(const bbsPublicKey *cpub, const bbsSignature *csig, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN, const uint32_t *discIdxs, uint32_t discN)
{
	uint32_t totalDiscMsgSize = 0;
	for (size_t i = 0; i < discN; i++) {
		totalDiscMsgSize += msgSize[discIdxs[i]];
	}
	uint8_t *discMsgs = (uint8_t*)CYBOZU_ALLOCA(totalDiscMsgSize);
	uint32_t *discMsgSize = (uint32_t*)CYBOZU_ALLOCA(sizeof(uint32_t) * discN);
	// copy msgs to discMsgSize
	copyMsgsToDisc(discMsgs, discMsgSize, msgs, msgSize, discIdxs, discN);

	const uint8_t nonce[] = { 1, 2, 3, 4, 0x11, 0x22, 0x33 };

	bbsProof *cprf = bbsCreateProof(cpub, csig, msgs, msgSize, msgN, discIdxs, discN, nonce, sizeof(nonce));
	CYBOZU_TEST_ASSERT(cprf);
	CYBOZU_TEST_ASSERT(bbsVerifyProof(cpub, cprf, msgN, discMsgs, discMsgSize, discIdxs, discN, nonce, sizeof(nonce)));

	{
		size_t n1 = bbsGetProofSerializeByteSize(cprf);
		char *buf = (char *)CYBOZU_ALLOCA(n1);

		size_t n = bbsSerializeProof(buf, n1, cprf);
		CYBOZU_TEST_ASSERT(n > 0);
		CYBOZU_TEST_EQUAL(n, n1);
		bbsProof *cprf2 = bbsDeserializeProof(buf, n);
		CYBOZU_TEST_ASSERT(cprf2);
		if (cprf2) {
			CYBOZU_TEST_ASSERT(bbsIsEqualProof(cprf, cprf2));
			bbsDestroyProof(cprf2);
		}
	}

	bbsDestroyProof(cprf);
}

CYBOZU_TEST_AUTO(proof)
{
	const size_t msgN = 10;
	uint32_t msgSize[msgN];
	uint32_t totalMsgSize = 0;
	for (size_t i = 0; i < msgN; i++) {
		msgSize[i] = 1 + ((i*15+i) % 13);
		totalMsgSize += msgSize[i];
	}
	uint8_t *msgs = (uint8_t*)CYBOZU_ALLOCA(totalMsgSize);
	for (size_t i = 0; i < totalMsgSize; i++) {
		msgs[i] = uint8_t(i*i+123*i+21);
	}

	uint32_t discIdxs[msgN];

	PublicKey pub;
	Signature sig;
	// setup public key and signature
	{
		SecretKey sec;
		sec.init();
		sec.getPublicKey(pub);
		sig.sign(sec, pub, msgs, msgSize, msgN);
	}
	bbsPublicKey cpub;
	bbsSignature csig;
	bbsSecretKey csec;
	{
		bbsInitSecretKey(&csec);
		bbsGetPublicKey(&cpub, &csec);
		bbsSign(&csig, &csec, &cpub, msgs, msgSize, msgN);
	}

	puts("disclose nothing");
	checkProof(pub, sig, msgs, msgSize, msgN, 0, 0);
	ccheckProof(&cpub, &csig, msgs, msgSize, msgN, 0, 0);

	// serialize/deserialize test
	{
		bbsSecretKey csec2;
		bbsPublicKey cpub2;
		bbsSignature csig2;
		char buf[1024];
		size_t n, n2;

		n = bbsSerializeSecretKey(buf, sizeof(buf), &csec);
		CYBOZU_TEST_EQUAL(n, bbsGetSecretKeySerializeByteSize());
		n2 = bbsDeserializeSecretKey(&csec2, buf, n);
		CYBOZU_TEST_EQUAL(n, n2);
		CYBOZU_TEST_ASSERT(bbsIsEqualSecretKey(&csec, &csec2));

		n = bbsSerializePublicKey(buf, sizeof(buf), &cpub);
		CYBOZU_TEST_EQUAL(n, bbsGetPublicKeySerializeByteSize());
		n2 = bbsDeserializePublicKey(&cpub2, buf, n);
		CYBOZU_TEST_EQUAL(n, n2);
		CYBOZU_TEST_ASSERT(bbsIsEqualPublicKey(&cpub, &cpub2));

		n = bbsSerializeSignature(buf, sizeof(buf), &csig);
		CYBOZU_TEST_EQUAL(n, bbsGetSignatureSerializeByteSize());
		n2 = bbsDeserializeSignature(&csig2, buf, n);
		CYBOZU_TEST_EQUAL(n, n2);
		CYBOZU_TEST_ASSERT(bbsIsEqualSignature(&csig, &csig2));
	}

	puts("disclose one");
	{
		const uint32_t discN = 1;
		for (uint32_t i = 0; i < msgN; i++) {
			discIdxs[0] = i;
			checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
			ccheckProof(&cpub, &csig, msgs, msgSize, msgN, discIdxs, discN);
		}
	}
	puts("disclose two");
	{
		const uint32_t discN = 2;
		for (uint32_t i = 0; i < msgN; i++) {
			discIdxs[0] = i;
			for (uint32_t j = i + 1; j < msgN; j++) {
				discIdxs[1] = j;
				checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
				ccheckProof(&cpub, &csig, msgs, msgSize, msgN, discIdxs, discN);
			}
		}
	}
	puts("disclose three");
	{
		const uint32_t discN = 3;
		for (uint32_t i = 0; i < msgN; i++) {
			discIdxs[0] = i;
			for (uint32_t j = i + 1; j < msgN; j++) {
				discIdxs[1] = j;
				for (uint32_t k = j + 1; k < msgN; k++) {
					discIdxs[2] = k;
					checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
					ccheckProof(&cpub, &csig, msgs, msgSize, msgN, discIdxs, discN);
				}
			}
		}
	}
	puts("disclose all");
	{
		const uint32_t discN = msgN;
		for (uint32_t i = 0; i < discN; i++) discIdxs[i] = i;
		checkProof(pub, sig, msgs, msgSize, msgN, discIdxs, discN);
		ccheckProof(&cpub, &csig, msgs, msgSize, msgN, discIdxs, discN);
	}
}
