#pragma once
/**
	@file
	@brief BBS signature
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <mcl/bn_c384_256.h>

#define BBS_PRIVATE_KEY_SIZE sizeof(mclBnFr)
#define BBS_PUBLIC_KEY_SIZE sizeof(mclBnG2)
#define BBS_SIGNATURE_SIZE (sizeof(mclBnG1)+sizeof(mclBnFr)*2)
#define BBS_PROOF_SIZE (sizeof(mclBnG1)*3+sizeof(mclBnFr)*5+sizeof(mclSize)*2)

#ifdef __cplusplus
extern "C" {
#endif

struct bbsSecretKey {
	uint64_t v[BBS_PRIVATE_KEY_SIZE/8];
};

struct bbsPublicKey {
	uint64_t v[BBS_PUBLIC_KEY_SIZE/8];
};

struct bbsSignature {
	uint64_t v[BBS_SIGNATURE_SIZE/8];
};

struct bbsProof; // destructor is need

MCL_DLL_API bool bbsInit(mclSize maxMsgSize);

MCL_DLL_API bool bbsInitSecretKey(bbsSecretKey *sec);

MCL_DLL_API bool bbsGetPublicKey(bbsPublicKey *pub, const bbsSecretKey *sec);

/*
	signature generation function for byte array messages
	Input:
		sec: secret key
		pub: public key
		msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
		msgSize: array storing size of each message (msgSize[i] is size of msg[i])
		msgN: number of messages
	Output:
		sig: generated signature
	Return:
		true: success
*/
MCL_DLL_API bool bbsSign(bbsSignature *sig, const bbsSecretKey *sec, const bbsPublicKey *pub, const uint8_t *msgs, const mclSize *msgSize, mclSize msgN);

/*
	signature verification function for byte array messages
	Input:
		pub: public key
		sig: signature
		msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
		msgSize: array storing size of each message (msgSize[i] is size of msg[i])
		msgN: number of messages
	Return:
		true: success
*/
MCL_DLL_API bool bbsVerify(const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *msgs, const mclSize *msgSize, mclSize msgN);

MCL_DLL_API bbsProof *bbsCreateProof(const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *msgs, const mclSize *msgSize, size_t msgN, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize);

MCL_DLL_API void bbsDestroyProof(bbsProof *proof);

MCL_DLL_API bool bbsVerifyProof(const bbsPublicKey *pub, const bbsProof *prf, size_t msgN, const uint8_t *discMsgs, const mclSize *discMsgSize, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize);

#ifdef __cplusplus
} // extern "C"
#endif
