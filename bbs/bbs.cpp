#include "bbs.hpp"
#include "bbs.h"

using namespace bbs;

inline SecretKey *cast(bbsSecretKey *p) { return reinterpret_cast<SecretKey*>(p); }
inline const SecretKey *cast(const bbsSecretKey *p) { return reinterpret_cast<const SecretKey*>(p); }

inline PublicKey *cast(bbsPublicKey *p) { return reinterpret_cast<PublicKey*>(p); }
inline const PublicKey *cast(const bbsPublicKey *p) { return reinterpret_cast<const PublicKey*>(p); }

inline Signature *cast(bbsSignature *p) { return reinterpret_cast<Signature*>(p); }
inline const Signature *cast(const bbsSignature *p) { return reinterpret_cast<const Signature*>(p); }

inline Proof *cast(bbsProof *p) { return reinterpret_cast<Proof*>(p); }
inline const Proof *cast(const bbsProof *p) { return reinterpret_cast<const Proof*>(p); }


bool bbsInit(mclSize maxMsgSize)
{
	return bbs::init(maxMsgSize);
}

bool bbsInitSecretKey(bbsSecretKey *sec)
{
	cast(sec)->init();
	return true;
}

bool bbsGetPublicKey(bbsPublicKey *pub, const bbsSecretKey *sec)
{
	cast(sec)->getPublicKey(*cast(pub));
	return true;
}

bool bbsSign(bbsSignature *sig, const bbsSecretKey *sec, const bbsPublicKey *pub, const uint8_t *msgs, const mclSize *msgSize, mclSize msgN)
{
	return cast(sig)->sign(*cast(sec), *cast(pub), msgs, msgSize, msgN);
}

bool bbsVerify(const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *msgs, const mclSize *msgSize, mclSize msgN)
{
	return cast(sig)->verify(*cast(pub), msgs, msgSize, msgN);
}

bbsProof *bbsCreateProof(const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *msgs, const mclSize *msgSize, size_t msgN, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize)
{
	if (msgN < discN) return 0;
	const mclSize undiscN = msgN - discN;
	uint8_t *p = (uint8_t*)malloc(sizeof(bbs::Proof) + sizeof(mcl::Fr) * undiscN);
	if (p == 0) return 0;
	bbsProof *proof = (bbsProof*)p;
	mcl::Fr *fr = (mcl::Fr*)(p + sizeof(bbs::Proof));
	cast(proof)->set(fr, undiscN);
	if (bbs::proofGen(*cast(proof), *cast(pub), *cast(sig), msgs, msgSize, msgN, discIdxs, discN, nonce, nonceSize)) {
		return proof;
	}
	free(p);
	return 0;
}

void bbsDestroyProof(bbsProof *proof)
{
	free(proof);
}

bool bbsVerifyProof(const bbsPublicKey *pub, const bbsProof *prf, size_t msgN, const uint8_t *discMsgs, const mclSize *discMsgSize, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize)
{
	return bbs::proofVerify(*cast(pub), *cast(prf), msgN, discMsgs, discMsgSize, discIdxs, discN, nonce, nonceSize);
}

