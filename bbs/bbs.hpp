#pragma once
/*
	This file is for study. Not at the product level.
*/
#include <mcl/bls12_381.hpp>
#include <cybozu/sha2.hpp>
#include <cybozu/serializer.hpp>

namespace mcl { namespace bbs {

using namespace mcl;

static const size_t MAX_MSG_SIZE = 1024;
static size_t s_maxMsgSize;
static G1 s_gen[2 + MAX_MSG_SIZE];
static G1& s_Q1 = s_gen[0];
static G1& s_Q2 = s_gen[1];
static G1* s_H = &s_gen[2];
static G1 s_P1;
static G2 s_P2;

static const uint8_t s_dst[] = "MAP_MSG_TO_SCALAR_AS_HASH_";
static const size_t s_dstSize = sizeof(s_dst) - 1;

#define PUT(x) std::cout << #x "=" << x << std::endl;

namespace local {

struct Hash {
	cybozu::Sha256 h_;
	template<class T>
	Hash& operator<<(const T& t)
	{
		char buf[sizeof(T)];
		cybozu::MemoryOutputStream os(buf, sizeof(buf));
		cybozu::save(os, t);
		h_.update(buf, os.getPos());
		return *this;
	}
	template<class F>
	void get(F& x)
	{
		uint8_t md[32];
		h_.digest(md, sizeof(md), 0, 0);
		x.setArrayMask(md, sizeof(md));
	}
};

// out = hash(pk, n, s_Q1, s_Q2, s_H[0:n])
inline void calcDom(Fr& out, const G2& pk, size_t n)
{
	Hash hash;
	hash << pk << n << s_Q1 << s_Q2;
	for (size_t i = 0; i < n; i++) {
		hash << s_H[i];
	}
	hash.get(out);
}

// B = s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i]
inline void calcB(G1& B, const Fr& s, const Fr& dom, const Fr *msgs, size_t n)
{
	{
		Fr t[2] = { s, dom };
		G1::mulVec(B, &s_Q1, t, 2); // B = s_Q1 * s + s_Q2 * dom
	}
	G1 T;
	G1::mulVec(T, s_H, msgs, n); // T = sum_i s_H[i] * msgs[i]
	B += T;
	B += s_P1;
}

// write to out[0], ..., out[n-1]
template<class T>
void hash_to_scalar(Fr *out, const T& x, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		Hash hash;
		hash << x << i;
		hash.get(out[i]);
	}
}

// true if all discIdxs[i] < discIdxs[i+1] < msgN
inline bool isValidDiscIdx(size_t msgN, const uint32_t *discIdxs, size_t R)
{
	if (R == 0) return true;
	if (discIdxs[0] >= msgN) return false;
	for (size_t i = 1; i < R; i++) {
		if (!(discIdxs[i - 1] < discIdxs[i]) || discIdxs[i] >= msgN) return false;
	}
	return true;
}

// js[0:U] = [0:msgN] - discIdxs[0:R]
inline void setJs(uint32_t *js, size_t U, const uint32_t *discIdxs, size_t R)
{
	const size_t msgN = U + R;
	size_t v = 0;
	size_t dPos = 0;
	uint32_t next = dPos < R ? discIdxs[dPos++] : msgN;

	size_t jPos = 0;
	while (jPos < U) {
		if (v < next) {
			js[jPos++] = v;
		} else {
			next = dPos < R ? discIdxs[dPos++] : msgN;
		}
		v++;
	}
}

// return e(P1, Q) == e(P2, s_P2);
inline bool verifyMultiPairing(const G1& P1, const G1& P2, const G2& Q)
{
	G1 v1[2] = { P1, P2 };
	G2 v2[2] = { Q, -s_P2 };
	GT out;
	millerLoopVec(out, v1, v2, 2);
	finalExp(out, out);
	return out.isOne();
}

inline void msgToFr(Fr& x, const uint8_t *msg, uint32_t msgSize)
{
	uint8_t md[64];
	fp::expand_message_xmd(md, sizeof(md), msg, msgSize, s_dst, s_dstSize);
	bool b;
	x.setBigEndianMod(&b, md, sizeof(md));
	assert(b); (void)b;
}

// x : Fr array of size msgN.
// msgs : concatenation of all msg[i]. The size is a sum of msgSize[i].
// msgSize : array of size msgN. msgSize[i] is the size of msg[i].
inline void msgsToFr(Fr *x, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN)
{
	for (size_t i = 0; i < msgN; i++) {
		msgToFr(x[i], msgs, msgSize[i]);
		msgs += msgSize[i];
	}
}

} // mcl::bbs::local

void init(size_t maxMsgSize)
{
	if (maxMsgSize > MAX_MSG_SIZE) {
		throw cybozu::Exception("too large maxMsgSize") << maxMsgSize;
	}
	s_maxMsgSize = maxMsgSize;
	initPairing(BLS12_381);
	Fp::setETHserialization(true);
	Fr::setETHserialization(true);
	setMapToMode(MCL_MAP_TO_MODE_HASH_TO_CURVE);
	// setup generator
	mapToG1(s_P1, -1);
	mapToG2(s_P2, -2);
	s_P1.normalize();
	s_P2.normalize();

	mapToG1(s_Q1, 1);
	mapToG1(s_Q2, 2);
	s_Q1.normalize();
	s_Q2.normalize();
	for (size_t i = 0; i < maxMsgSize; i++) {
		mapToG1(s_H[i], 3 + i);
		s_H[i].normalize();
	}
}

class PublicKey {
	G2 v;
	friend class SecretKey;
public:
	const G2& get_v() const { return v; }
};

class SecretKey {
	Fr v;
public:
	void initForDebug(int v)
	{
		this->v = v;
	}
	void init()
	{
		v.setByCSPRNG();
	}
	void getPublicKey(PublicKey& pub) const
	{
		G2::mul(pub.v, s_P2, v);
	}
	const Fr& get_v() const { return v; }
};

class Signature {
	G1 A;
	Fr e, s;
	friend class SecretKey;
public:
	const G1& get_A() const { return A; }
	const Fr& get_e() const { return e; }
	const Fr& get_s() const { return s; }
	// msgN : number of msgs
	bool sign(const SecretKey& sec, const PublicKey& pub, const Fr *msgs, size_t msgN);
	bool verify(const PublicKey& pub, const Fr *msgs, size_t msgN) const;

	// msgs : concatinate of msg[0], ..., msg[msgN-1]
	// msgN : amount of msg
	// msgSize : array of each msg[i] size
	bool sign(const SecretKey& sec, const PublicKey& pub, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN)
	{
		Fr *xs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * msgN);
		local::msgsToFr(xs, msgs, msgSize, msgN);
		return sign(sec, pub, xs, msgN);
	}
	bool verify(const PublicKey& pub, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN) const
	{
		Fr *xs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * msgN);
		local::msgsToFr(xs, msgs, msgSize, msgN);
		return verify(pub, xs, msgN);
	}
};

/*
	e, s : generated from msgs
	B = s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i]
	A = (1/(sec + e))B
	return (A, e, s)
	assume msgN <= s_maxMsgSize
*/
inline bool Signature::sign(const SecretKey& sec, const PublicKey& pub, const Fr *msgs, size_t msgN)
{
	if (msgN > s_maxMsgSize) {
		return false;
	}
	Fr dom;
	local::calcDom(dom, pub.get_v(), msgN);
	local::Hash hash;
	hash << sec.get_v() << dom;
	for (size_t i = 0; i < msgN; i++) {
		hash << msgs[i];
	}
	Fr t;
	hash.get(t);
	Fr out[2];
	local::hash_to_scalar(out, t, 2);
	e = out[0];
	s = out[1];
	G1 B;
	local::calcB(B, s, dom, msgs, msgN);
	Fr::add(t, sec.get_v(), e);
	Fr::inv(t, t);
	G1::mul(A, B, t);
	return true;
}

/*
	dom = hash(pk, n, s_Q1, s_Q2, s_H[0:n])
	e(A, pub + e * s_P2) = e(s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i], s_P2)
*/
inline bool Signature::verify(const PublicKey& pub, const Fr *msgs, size_t msgN) const
{
	if (msgN > s_maxMsgSize) return false;
	Fr dom;
	local::calcDom(dom, pub.get_v(), msgN);
	G1 B;
	local::calcB(B, s, dom, msgs, msgN);
	return local::verifyMultiPairing(A, B, pub.get_v() + s_P2 * e);
}

struct Proof {
	G1 A_prime, A_bar, D;
	Fr c, e_hat, r2_hat, r3_hat, s_hat;
	Fr *m_hat; // m_hat must be U array of Fr
	uint32_t U; // U = msgN - R, all msgs are disclosed if U = 0
	Proof() : m_hat(0), U(0) {}
	void set(Fr *msg, uint32_t u)
	{
		m_hat = msg;
		U = u;
	}
};

namespace local {

inline void calc_cv(Fr& cv, const Proof& prf, const G1& C1, const G1& C2, size_t R, const uint32_t *discIdxs, const Fr *msgs, bool isDisclosed, const Fr& dom, const uint8_t *nonce = 0, size_t nonceSize = 0)
{
	local::Hash hash;
	for (size_t i = 0; i < nonceSize; i++) hash << nonce[i];
	hash << prf.A_prime << prf.A_bar << prf.D << C1 << C2 << R;
	for (size_t i = 0; i < R; i++) hash << discIdxs[i];
	if (isDisclosed) {
		// disclosed msg
		for (size_t i = 0; i < R; i++) hash << msgs[i];
	} else {
		// select disclosed msg
		for (size_t i = 0; i < R; i++) hash << msgs[discIdxs[i]];
	}
	hash << dom;
	hash.get(cv);
}

// out += sum_{i=0}^U s_H[selectedIx[i]] * v[i]
inline void addSelectedMulVec(G1& out, const uint32_t *selectedIdx, size_t U, const Fr *v)
{
	G1 *H = (G1*)CYBOZU_ALLOCA(sizeof(G1) * U);
	for (size_t i = 0; i < U; i++) H[i] = s_H[selectedIdx[i]];
	G1 T;
	G1::mulVec(T, H, v, U);
	out += T;
}

} // local
/*
	msgN : number of all msgs
	R : number of disclosed msgs
	discIdxs : accending order
	msgs[discIdxs[i]] : disclosed messages for i in [0, R)
*/
inline bool proofGen(Proof& prf, const PublicKey& pub, const Signature& sig, const Fr *msgs, size_t msgN, const uint32_t *discIdxs, size_t R, const uint8_t *nonce = 0, size_t nonceSize = 0)
{
	if (msgN > s_maxMsgSize) return false;
	if (msgN < R) return false;
	if (prf.U != msgN - R) return false;
	if (!local::isValidDiscIdx(msgN, discIdxs, R)) return false;
	Fr dom;
	local::calcDom(dom, pub.get_v(), msgN);
	Fr out[6];
	local::hash_to_scalar(out, 0, 6);
	const Fr& r1 = out[0];
	const Fr& r2 = out[1];
	const Fr& e_tilde = out[2];
	const Fr& r2_tilde = out[3];
	const Fr& r3_tilde = out[4];
	const Fr& s_tilde = out[5];
	G1 B;
	local::calcB(B, sig.get_s(), dom, msgs, msgN);
	Fr r3;
	Fr::inv(r3, r1);
	prf.A_prime = sig.get_A() * r1;
	prf.A_bar = prf.A_prime * (-sig.get_e()) + B * r1;
	prf.D = B * r1 + s_Q1 * r2;
	Fr s_prime;
	s_prime = r2 * r3 + sig.get_s();
	G1 C1, C2;
	C1 = prf.A_prime * e_tilde + s_Q1 * r2_tilde;
	C2 = prf.D * (-r3_tilde) + s_Q1 * s_tilde;

	uint32_t *js = (uint32_t*)CYBOZU_ALLOCA(sizeof(uint32_t) * prf.U);
	local::setJs(js, prf.U, discIdxs, R);
	Fr *m_tilde = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * prf.U);
	local::hash_to_scalar(m_tilde, "m_tilde", prf.U);
	if (prf.U > 0) {
		local::addSelectedMulVec(C2, js, prf.U, m_tilde);
	}
	local::calc_cv(prf.c, prf, C1, C2, R, discIdxs, msgs, false, dom, nonce, nonceSize);
	prf.e_hat= prf.c * sig.get_e() + e_tilde;
	prf.r2_hat = prf.c * r2 + r2_tilde;
	prf.r3_hat = prf.c * r3 + r3_tilde;
	prf.s_hat = prf.c * s_prime + s_tilde;

	if (prf.U > 0) {
		for (size_t i = 0; i < prf.U; i++) {
			prf.m_hat[i] = prf.c * msgs[js[i]] + m_tilde[i];
		}
	}
	return true;
}

bool proofVerify(const PublicKey& pub, const Proof& prf, size_t msgN, const Fr *discMsgs, const uint32_t *discIdxs, size_t R, const uint8_t *nonce = 0, size_t nonceSize = 0)
{
	if (msgN > s_maxMsgSize) return false;
	if (msgN < R) return false;
	if (prf.U != msgN - R) return false;
	if (!local::isValidDiscIdx(msgN, discIdxs, R)) return false;
	Fr dom;
	local::calcDom(dom, pub.get_v(), msgN);
	G1 C1 = (prf.A_bar - prf.D) * prf.c + prf.A_prime * prf.e_hat + s_Q1 * prf.r2_hat;
	G1 T = s_P1 + s_Q2 * dom;
	if (R > 0) {
		local::addSelectedMulVec(T, discIdxs, R, discMsgs);
	}
	G1 C2 = T * prf.c - prf.D * prf.r3_hat + s_Q1 * prf.s_hat;
	if (prf.U > 0) {
		uint32_t *js = (uint32_t*)CYBOZU_ALLOCA(sizeof(uint32_t) * prf.U);
		local::setJs(js, prf.U, discIdxs, R);
		local::addSelectedMulVec(C2, js, prf.U, prf.m_hat);
	}
	Fr cv;
	local::calc_cv(cv, prf, C1, C2, R, discIdxs, discMsgs, true, dom, nonce, nonceSize);
	if (cv != prf.c) return false;
	if (prf.A_prime.isZero()) return false;
	return local::verifyMultiPairing(prf.A_prime, prf.A_bar, pub.get_v());
}

} } // mcl::bbs

