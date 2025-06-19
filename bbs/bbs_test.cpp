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
		dst[n/2 - 1 - i/2] = L | (H << 4);
	}
	return n/2;
}

/*
secHex=6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811
sec 6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811
pub 12bc23fcfd1fc4b4d5c563d4c626221c3386a30ce7a10a3d049b1e1bf7df88f5c1db95038ab61e88692abe4ca9ed2a1175ee09d105280c65033060ca975277cda30d40351d14135cedd4b3133f2c4dd853de48d22e187a6ab8a62a11bc134f92
msgs=vkbvqnmnqvbkvhwmezvttvzemwhvkbvqnmnqv
msgs[0]={"v", 1}
msgs[1]={"kbv", 3}
msgs[2]={"qnmnq", 5}
msgs[3]={"vbkvhwm", 7}
msgs[4]={"ez", 2}
msgs[5]={"vttv", 4}
msgs[6]={"zemwhv", 6}
msgs[7]={"k", 1}
msgs[8]={"bvq", 3}
msgs[9]={"nmnqv", 5}
sig 393bafd6e0839f04ea907ad450a93f0aa094d9e157d3bba890de3883d7a0a943467a8d10897f16734959b960ffc527959c1eecb3b0778616e4a30375481aa408a9a1f8f0ae474e4bd83a7143cdd6cf8db82519ea63b2a17bc042e96c52e81ab8fe130bb04cc401ac0687d5cc69309394
nonce 221109
prf b1647e15d22260e432e3e376d71c4e2eef6e5d33a9cbfa758600d702d4ed3f4341e32793eb645911f3e06eb2d7356d6594be8e505e021eb4103fbc5bfe18e320b75c681d45fa2b16419e74d3932812d1030707a46dc2b7368fbcc20d10bfd731f09df1cadac49df778b6ed7d11978e391bb89031f00018bef43641df13cc9e4ffa2673d91ebea34705d332c8362f40821986cd8b3effe29f8da63bdaaeff744265200d4191c5c2f88bacc9ad5c8baa4e7dda676286c45637f038610f8807bb53d3e041333ab74de8a8712e88882872bac399b325ee6f2a211dd856b032a90145000000070000000af1aacdeb18ef9ba04b605e8c91f14ead63dfa86c5747b0d4f73fc6db03deb72df4313a903ec6a32cca5226183c5df19f4a790edef0dd2ad519c423e4b7311c62ed9b574ac48ed1046adf95637a915e656672e893d1800823fa43c54066639d16edd28b0c5938ab813e25169baca85b2bab9736cbf122e7d642f5fffbefec482d9784cab00893f6436e1303f30b03081d9de3d015e2eae2ad370b8b7194f48c3e12a49d028e4de0d02fdef0df48cb5eaac44db8ceeaf40f27849fe5501ab10a524678da9151a73833c9fd9f2fbbb925a09b51f9ae0ce652eaf7e6e958aae2b0d04f8d80056e730e451a1ef987320dabf8397b1696a1e4ce509f4ef24f2ca911aa8c232876de42ec4c1659b6af91e43e096bf39f20c25b230d3fea4caa69c42dea58d6f102d6e18d1a947041d91d2c3daf
discMsgs 767474767a6576626b
*/
CYBOZU_TEST_AUTO(proof2)
{
	uint8_t buf[1024];
	size_t n;

	const char *secHex = "6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811";
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
	mcl::bint::dump(buf, n, "sec");
	bbsGetPublicKey(&pub, &sec);
	n = bbsSerializePublicKey(buf, sizeof(buf), &pub);
	mcl::bint::dump(buf, n, "pub");

	const uint32_t msgN = 10;
	uint32_t msgSize[msgN];
	uint32_t totalMsgSize = 0;
	for (size_t i = 0; i < msgN; i++) {
		msgSize[i] = 1 + ((i*15+i) % 7);
		totalMsgSize += msgSize[i];
	}
	uint8_t *msgs = (uint8_t*)CYBOZU_ALLOCA(totalMsgSize);
	for (size_t i = 0; i < totalMsgSize; i++) {
		msgs[i] = uint8_t('a' + ((i*i+123*i+21)%27));
	}
	printf("msgs=%s\n", msgs);
	uint32_t pos = 0;
	for (uint32_t i = 0; i < msgN; i++) {
		printf("msgs[%d]={\"", i);
		for (uint32_t j = 0; j < msgSize[i]; j++) {
			printf("%c", msgs[pos + j]);
		}
		printf("\", %u}\n", msgSize[i]);
		pos += msgSize[i];
	}
	CYBOZU_TEST_ASSERT(bbsSign(&sig, &sec, &pub, msgs, msgSize, msgN));
	n = bbsSerializeSignature(buf, sizeof(buf), &sig);
	mcl::bint::dump(buf, n, "sig");

	uint32_t discIdxs[] = { 1, 4, 5 };
	const uint32_t discN = uint32_t(sizeof(discIdxs) / sizeof(discIdxs[0]));
	const uint8_t nonce[] = { 9, 0x11, 0x22 };
	const uint32_t nonceSize = uint32_t(sizeof(nonce));
	mcl::bint::dump(nonce, nonceSize, "nonce");
	bbsProof *prf = bbsCreateProof(&pub, &sig, msgs, msgSize, msgN, discIdxs, discN, nonce, nonceSize);
	CYBOZU_TEST_ASSERT(prf);
	n = bbsSerializeProof(buf, sizeof(buf), prf);
	CYBOZU_TEST_ASSERT(n > 0);
	mcl::bint::dump(buf, n, "prf");

	uint32_t totalDiscMsgSize = 0;
	for (size_t i = 0; i < discN; i++) {
		totalDiscMsgSize += msgSize[discIdxs[i]];
	}
	uint8_t *discMsgs = (uint8_t*)CYBOZU_ALLOCA(totalDiscMsgSize);
	uint32_t *discMsgSize = (uint32_t*)CYBOZU_ALLOCA(sizeof(uint32_t) * discN);
	// copy msgs to discMsgSize
	copyMsgsToDisc(discMsgs, discMsgSize, msgs, msgSize, discIdxs, discN);
	mcl::bint::dump(discMsgs, totalDiscMsgSize, "discMsgs");
	CYBOZU_TEST_ASSERT(bbsVerifyProof(&pub, prf, discMsgs, discMsgSize, discIdxs, discN, nonce, nonceSize));
}
