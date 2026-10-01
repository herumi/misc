#pragma once
/**
	@file
	@brief BBS signature (draft-irtf-cfrg-bbs-signatures-12)
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <mcl/bn_c384_256.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	BBS_BLS12381_SHA256 = 0, // BBS_BLS12381G1_XMD:SHA-256_SSWU_RO_
	BBS_BLS12381_SHAKE256 = 1 // BBS_BLS12381G1_XOF:SHAKE-256_SSWU_RO_ (not supported yet)
};

struct bbsSecretKey {
	mclBnFr v;
};

struct bbsPublicKey {
	mclBnG2 v;
};

struct bbsSignature {
	mclBnG1 A;
	mclBnFr e;
};

/*
	size of the structures
	These types are arrays of integers without padding, so a binding of another language has only to know the sizes.
*/
MCL_DLL_API mclSize bbsSizeofSecretKey();
MCL_DLL_API mclSize bbsSizeofPublicKey();
MCL_DLL_API mclSize bbsSizeofSignature();

/*
	size of the octet strings defined by the spec
	secret key: 32, public key: 96, signature: 80
*/
MCL_DLL_API mclSize bbsGetSecretKeySerializeByteSize();
MCL_DLL_API mclSize bbsGetPublicKeySerializeByteSize();
MCL_DLL_API mclSize bbsGetSignatureSerializeByteSize();
/*
	size of a proof for undiscN undisclosed messages
	3 * 48 + (4 + undiscN) * 32
*/
MCL_DLL_API mclSize bbsGetProofSize(uint32_t undiscN);

/*
	deserialize
	return read size if success else 0
	bbsDeserializeSecretKey fails if the value is zero or not less than r.
	bbsDeserializePublicKey fails if the point is the identity or not in G2.
	bbsDeserializeSignature fails if A is the identity or not in G1, or e is zero.
*/
MCL_DLL_API mclSize bbsDeserializeSecretKey(bbsSecretKey *x, const void *buf, mclSize bufSize);
MCL_DLL_API mclSize bbsDeserializePublicKey(bbsPublicKey *x, const void *buf, mclSize bufSize);
MCL_DLL_API mclSize bbsDeserializeSignature(bbsSignature *x, const void *buf, mclSize bufSize);

/*
	serialize
	return written byte if sucess else 0
*/
MCL_DLL_API mclSize bbsSerializeSecretKey(void *buf, mclSize maxBufSize, const bbsSecretKey *x);
MCL_DLL_API mclSize bbsSerializePublicKey(void *buf, mclSize maxBufSize, const bbsPublicKey *x);
MCL_DLL_API mclSize bbsSerializeSignature(void *buf, mclSize maxBufSize, const bbsSignature *x);

MCL_DLL_API bool bbsIsEqualSecretKey(const bbsSecretKey *lhs, const bbsSecretKey *rhs);
MCL_DLL_API bool bbsIsEqualPublicKey(const bbsPublicKey *lhs, const bbsPublicKey *rhs);
MCL_DLL_API bool bbsIsEqualSignature(const bbsSignature *lhs, const bbsSignature *rhs);

/*
	initialize the library
	cipherSuite: BBS_BLS12381_SHA256
	maxMsgN: max number of messages. maxMsgN + 1 generators are computed and cached.
	Sign/Verify/ProofGen/ProofVerify fail if the number of messages is larger than maxMsgN.
	bbsInit can be called again with a larger maxMsgN to extend the generators.
	bbsInit and bbsTerm are not thread safe. Do not call them while other functions are running.
	Return:
		true: success
*/
MCL_DLL_API bool bbsInit(int cipherSuite, uint32_t maxMsgN);

/*
	free the generators allocated by bbsInit
	It is safe to call it twice. bbsInit can be called again after bbsTerm.
*/
MCL_DLL_API void bbsTerm();

/*
	KeyGen of the spec
	Input:
		keyMaterial: secret octet string. keyMaterialSize >= 32
		keyInfo: optional (NULL with keyInfoSize = 0). keyInfoSize <= 65535
		keyDst: optional. the default (ciphersuite_id || "KEYGEN_DST_") is used if keyDst is NULL.
	Output:
		sec: secret key
	Return:
		true: success
*/
MCL_DLL_API bool bbsKeyGen(bbsSecretKey *sec, const uint8_t *keyMaterial, mclSize keyMaterialSize, const uint8_t *keyInfo, mclSize keyInfoSize, const uint8_t *keyDst, mclSize keyDstSize);

// generate a secret key by CSPRNG
MCL_DLL_API bool bbsInitSecretKey(bbsSecretKey *sec);

// SkToPk of the spec
MCL_DLL_API bool bbsGetPublicKey(bbsPublicKey *pub, const bbsSecretKey *sec);

/*
	Sign of the spec
	Input:
		sec: secret key
		pub: public key
		header: optional (NULL with headerSize = 0)
		msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
		msgSize: array storing size of each message (msgSize[i] is size of msg[i])
		msgN: number of messages
	Output:
		sig: generated signature
	Return:
		true: success
*/
MCL_DLL_API bool bbsSign(bbsSignature *sig, const bbsSecretKey *sec, const bbsPublicKey *pub, const uint8_t *header, mclSize headerSize, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN);

/*
	Verify of the spec
	Input:
		sig: signature
		pub: public key
		header: optional (NULL with headerSize = 0)
		msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
		msgSize: array storing size of each message (msgSize[i] is size of msg[i])
		msgN: number of messages
	Return:
		true: valid
*/
MCL_DLL_API bool bbsVerify(const bbsSignature *sig, const bbsPublicKey *pub, const uint8_t *header, mclSize headerSize, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN);

/*
	ProofGen of the spec
	Input:
		maxProofSize: size of the buffer proof. It must be at least bbsGetProofSize(msgN - discN).
		pub: public key
		sig: signature for (header, msgs). This function does not verify sig, so call bbsVerify in advance if it is untrusted.
		header: the header used at bbsSign (NULL with headerSize = 0 if not used)
		ph: presentation header. optional (NULL with phSize = 0)
		msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
		msgSize: array storing size of each message (msgSize[i] is size of msg[i])
		msgN: number of messages
		discIdxs: indexes of the disclosed messages in strictly ascending order
		discN: number of the disclosed messages
	Output:
		proof: generated proof
	Return:
		written size if success else 0
*/
MCL_DLL_API mclSize bbsProofGen(uint8_t *proof, mclSize maxProofSize, const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *header, mclSize headerSize, const uint8_t *ph, mclSize phSize, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN, const uint32_t *discIdxs, uint32_t discN);

/*
	ProofVerify of the spec
	Input:
		pub: public key
		proof: proof generated by bbsProofGen
		header: the header used at bbsSign (NULL with headerSize = 0 if not used)
		ph: the presentation header used at bbsProofGen (NULL with phSize = 0 if not used)
		discMsgs: concatenated disclosed message byte array (msg[discIdxs[0]] || ... || msg[discIdxs[discN-1]])
		discMsgSize: array storing size of each disclosed message
		discIdxs: indexes of the disclosed messages in strictly ascending order
		discN: number of the disclosed messages
	The number of the signed messages is computed from proofSize and discN.
	Return:
		true: valid
*/
MCL_DLL_API bool bbsProofVerify(const bbsPublicKey *pub, const uint8_t *proof, mclSize proofSize, const uint8_t *header, mclSize headerSize, const uint8_t *ph, mclSize phSize, const uint8_t *discMsgs, const uint32_t *discMsgSize, const uint32_t *discIdxs, uint32_t discN);

#ifdef __cplusplus
} // extern "C"
#endif
