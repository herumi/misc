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
	prf.set(msgN, undiscN, m_hat);
	const uint8_t nonce[] = { 1, 2, 3, 4, 0x11, 0x22, 0x33 };
	CYBOZU_TEST_ASSERT(proofGen(prf, pub, sig, msgs, msgSize, msgN, discIdxs, discN, nonce, sizeof(nonce)));
	CYBOZU_TEST_ASSERT(proofVerify(pub, prf, discMsgs, discMsgSize, discIdxs, discN, nonce, sizeof(nonce)));
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
	CYBOZU_TEST_ASSERT(bbsVerifyProof(cpub, cprf, discMsgs, discMsgSize, discIdxs, discN, nonce, sizeof(nonce)));

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
		printf("sec=%s\n", sec.get_v().getStr(16).c_str());
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

size_t hex2u8(uint8_t *dst, size_t maxSize, const char *hex, size_t n)
{
	if (n % 2) throw cybozu::Exception("n is odd") << n;
	if (maxSize < n/2) throw cybozu::Exception("maxSize is small") << maxSize << n;
	for (size_t i = 0; i < n; i += 2) {
		uint8_t L, H;
		if (!mcl::fp::local::hexCharToUint8(&H, hex[i])) throw cybozu::Exception("bad char H") << i << hex[i];
		if (!mcl::fp::local::hexCharToUint8(&L, hex[i+1])) throw cybozu::Exception("bad char L") << i << hex[i+1];
		dst[i/2] = L | (H << 4);
	}
	return n/2;
}

void dump(const void *_buf, size_t n, const char *msg = "")
{
	const uint8_t *buf = (const uint8_t*)_buf;
	if (msg) printf("%s=", msg);
	printf(" %zd ", n);
	for (size_t i = 0; i < n; i++) {
		printf("%02x", buf[i]);
	}
	printf("\n");
}

/*
secHex=6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811
sec= 32 6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811
pub= 96 935ccf9167e9dce6abd9746a0ea64b3254e6d4ccdc4380a7eca207347ae834acaec4476ab12c6f89f443ac8020b57d4d017d58c38207aa5aa7032d3f4b7f02c590784ae24ff6847d75d2d5e122210a849c9c9fcb5638dfe95069e409d6ccb3d5
sig= 112 865946c48d8fb1a59a18bedd3da6bd19291794ad42819da3051952f744fe9895d1115cdef578bbd19ed7362ba458ffb0080dca8f2c92936688e205f4b8788088651785744fd18fe3b7024d22e3918d2f663649206519fd85b7e463f9d1f7cce1d82829d496e96deb35aa75db5c3ae375
nonce= 3 091122
prf= 536 adece52c8d97bb1a1e2242ffa4f4fb53c03039d0c91c488e60cb05f925b620785b59154a1c9c4aff91a6bc8eba62e7ef91dfc0dd3ec317193afd9ed0e2f46bbcfc6417ea9ee82b768c18ebe87eb547e92d1f442f1a2cfbb6a58df748c6ca9d43b875587d47b62853e21a69d5cfa5f12ecd99a09f0dd0877cbd8d77208cf264697e0251d96821cae44e4ff6ef5a2cf4583ba2dd883f147c00c9e10b2674fd67847128fcff44e7ca424b139855d356dd2305ab153230eb281ca948cb81abe59759153fd3e7d722f8486fb87d45d50ac9713b41334cdd5e5ef701331570e572114537fa45b1e3b53362c37de8da338012403f548d96a03509ff209b0115e8da1818deb478c32d184e4b2ad04952acac5d6e06328048227bb6477e87d9467d47247b983342346a9e5bfced3599efab8002fd0a00000007000000170040db798627934fb2604610f6c08bf8b5ed0589d9a397b751565e49756ea32a464d2a3f0b88e4039786950d13e589c9dc827e0fd1fa5991c38939fe34cebb125300462d019e2d28fdbb7b21e1566ab2c86ad7d7b3dd5cffeb2279e100953d31451b09036a4d72a55561c5281749399be11b6699bf1f7ab1710c406bd0a70f6ab1a6d4fd8cb787c87fc71f0dce466ad9ced0660ba453728c40054db3da52f01cdbf60dcb330cb24d9d4edb2ced3c11482bd494ba7c14fce49cbb734281eb675a7b9f91b8d5ad9616241bc767d570dfcae3c1abe5e30ba6da0733f0652dcff7
discMsgs= 9 6b6276657a76747476
*/
CYBOZU_TEST_AUTO(proof2)
{
	uint8_t buf[1024];
	size_t n;

	const char *secHex = "6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811";
//	const char *secHex = "0000000000000000000000000000000000000000000000000000000000000001";
printf("secHex=%s\n", secHex);
	n = hex2u8(buf, sizeof(buf), secHex, strlen(secHex));
	bbsSecretKey sec;
	bbsPublicKey pub;
	bbsSignature sig;
#if 1
	n = bbsDeserializeSecretKey(&sec, buf, n);
	CYBOZU_TEST_ASSERT(n > 0);
#else
	bbsInitSecretKey(&sec);
	n = bbsSerializeSecretKey(buf, sizeof(buf), &sec);
#endif
	dump(buf, n, "sec");
	bbsGetPublicKey(&pub, &sec);
	n = bbsSerializePublicKey(buf, sizeof(buf), &pub);
	dump(buf, n, "pub");

	const char *msgTbl[] = {
		"v", "kbv", "qnmnq", "vbkvhwm", "ez", "vttv", "zemwhv", "k", "bvq", "nmnqv"
	};
	const uint32_t msgN = uint32_t(sizeof(msgTbl)/sizeof(msgTbl[0]));
	uint32_t msgSize[msgN];
	uint32_t totalMsgSize = 0;
	for (size_t i = 0; i < msgN; i++) {
		msgSize[i] = uint32_t(strlen(msgTbl[i]));
		totalMsgSize += msgSize[i];
	}
	uint8_t *msgs = (uint8_t*)CYBOZU_ALLOCA(totalMsgSize);
	uint32_t pos = 0;
	for (size_t i = 0; i < msgN; i++) {
		memcpy(msgs + pos, msgTbl[i], msgSize[i]);
		pos += msgSize[i];
	}
	CYBOZU_TEST_ASSERT(bbsSign(&sig, &sec, &pub, msgs, msgSize, msgN));
	n = bbsSerializeSignature(buf, sizeof(buf), &sig);
	dump(buf, n, "sig");

	uint32_t discIdxs[] = { 1, 4, 5 };
	const uint32_t discN = uint32_t(sizeof(discIdxs) / sizeof(discIdxs[0]));
	const uint8_t nonce[] = { 9, 0x11, 0x22 };
	const uint32_t nonceSize = uint32_t(sizeof(nonce));
	dump(nonce, nonceSize, "nonce");
	dump(msgs, totalMsgSize, "msgs");
	dump(msgSize, msgN*4, "msgSize");
	dump(discIdxs, discN*4, "discIdxs");
	bbsProof *prf = bbsCreateProof(&pub, &sig, msgs, msgSize, msgN, discIdxs, discN, nonce, nonceSize);
	CYBOZU_TEST_ASSERT(prf);
	n = bbsSerializeProof(buf, sizeof(buf), prf);
	CYBOZU_TEST_ASSERT(n > 0);
	dump(buf, n, "prf");

	uint32_t totalDiscMsgSize = 0;
	for (size_t i = 0; i < discN; i++) {
		totalDiscMsgSize += msgSize[discIdxs[i]];
	}
	uint8_t *discMsgs = (uint8_t*)CYBOZU_ALLOCA(totalDiscMsgSize);
	uint32_t *discMsgSize = (uint32_t*)CYBOZU_ALLOCA(sizeof(uint32_t) * discN);
	// copy msgs to discMsgSize
	copyMsgsToDisc(discMsgs, discMsgSize, msgs, msgSize, discIdxs, discN);
	dump(discMsgs, totalDiscMsgSize, "discMsgs");
	dump(discMsgSize, discN*4, "discMsgSize");
	dump(discIdxs, discN*4, "discIdxs");
	dump(nonce, nonceSize, "nonce");
	CYBOZU_TEST_ASSERT(bbsVerifyProof(&pub, prf, discMsgs, discMsgSize, discIdxs, discN, nonce, nonceSize));
}
