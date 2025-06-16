#pragma once
/*
	This file is for study. Not at the product level.
*/
#include <mcl/bls12_381.hpp>
#include <bbs.h>

namespace bbs {

bool init(size_t maxMsgSize);

class PublicKey {
	bbsPublicKey v;
	friend class SecretKey;
public:
	const mcl::G2& get_v() const;
};

class SecretKey {
	bbsSecretKey v;
public:
	void init();
	void getPublicKey(PublicKey& pub) const;
	const mcl::Fr& get_v() const;
};

class Signature {
	bbsSignature v;
public:
	// msgN: number of msgs
	bool sign(const SecretKey& sec, const PublicKey& pub, const mcl::Fr *msgs, size_t msgN);
	bool verify(const PublicKey& pub, const mcl::Fr *msgs, size_t msgN) const;

	/*
		BBS+ signature generation function for byte array messages
		Input:
			sec: secret key
			pub: public key
			msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
			msgSize: array storing size of each message (msgSize[i] is size of msg[i])
			msgN: number of messages
		Return:
			true: success
	*/
	bool sign(const SecretKey& sec, const PublicKey& pub, const uint8_t *msgs, const mclSize *msgSize, size_t msgN);
	/*
		BBS+ signature verification function for byte array messages
		Input:
			pub: public key
			msgs: concatenated message byte array (msg[0] || msg[1] || ... || msg[msgN-1])
			msgSize: array storing size of each message (msgSize[i] is size of msg[i])
			msgN: number of messages
		Return:
			true: success
	*/
	bool verify(const PublicKey& pub, const uint8_t *msgs, const mclSize *msgSize, size_t msgN) const;

	const mcl::G1& get_A() const;
	const mcl::Fr& get_e() const;
	const mcl::Fr& get_s() const;
};

struct Proof {
	mcl::G1 A_prime, A_bar, D;
	mcl::Fr c, e_hat, r2_hat, r3_hat, s_hat;
	mclSize undiscN; // undiscN = msgN - discN, all msgs are disclosed if undiscN = 0
	mcl::Fr *m_hat; // m_hat must be undiscN array of Fr
	Proof(): undiscN(0), m_hat(0) {}
	void set(mcl::Fr *msg, mclSize undiscN)
	{
		m_hat = msg;
		this->undiscN = undiscN;
	}
};

/*
	msgN: number of all msgs
	discN: number of disclosed msgs
	discIdxs: accending order
	msgs[discIdxs[i]]: disclosed messages for i in [0, discN)
*/
bool proofGen(Proof& prf, const PublicKey& pub, const Signature& sig, const mcl::Fr *msgs, size_t msgN, const mclSize *discIdxs, size_t discN, const uint8_t *nonce = 0, size_t nonceSize = 0);

bool proofGen(Proof& prf, const PublicKey& pub, const Signature& sig, const uint8_t *msgs, const mclSize *msgSize, size_t msgN, const mclSize *discIdxs, size_t discN, const uint8_t *nonce = 0, size_t nonceSize = 0);

bool proofVerify(const PublicKey& pub, const Proof& prf, size_t msgN, const mcl::Fr *discMsgs, const mclSize *discIdxs, size_t discN, const uint8_t *nonce = 0, size_t nonceSize = 0);

bool proofVerify(const PublicKey& pub, const Proof& prf, size_t msgN, const uint8_t *discMsgs, const mclSize *discMsgSize, const mclSize *discIdxs, size_t discN, const uint8_t *nonce = 0, size_t nonceSize = 0);

namespace local {

// js[0:undiscN] = [0:msgN] - discIdxs[0:discN]
void setJs(mclSize *js, size_t undiscN, const mclSize *discIdxs, size_t discN);

} // bbs::local

} // bbs
