#pragma once
/*
	This file is for study. Not at the product level.
*/
#include <mcl/bls12_381.hpp>
#include <cybozu/sha2.hpp>
#include <cybozu/serializer.hpp>

namespace mcl { namespace bbs {

using namespace mcl;
using namespace mcl::bn;

static const size_t MAX_MSG_SIZE = 1024;
static size_t s_maxMsgSize;
static G1 s_gen[2 + MAX_MSG_SIZE];
static G1& s_Q1 = s_gen[0];
static G1& s_Q2 = s_gen[1];
static G1* s_H = &s_gen[2];
static G1 s_P1;
static G2 s_P2;

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
void calcDom(Fr& out, const G2& pk, size_t n)
{
	Hash hash;
	hash << pk << n << s_Q1 << s_Q2;
	for (size_t i = 0; i < n; i++) {
		hash << s_H[i];
	}
	hash.get(out);
}

// B = s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i]
void calcB(G1& B, const Fr& s, const Fr& dom, const Fr *msgs, size_t n)
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

class Signature {
	G1 A;
	Fr e, s;
	friend class SecretKey;
public:
	// L : number of msgs
	bool verify(const PublicKey& pub, const Fr *msgs, size_t L) const;
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
	// L : number of msgs
	void sign(Signature& sig, const PublicKey& pub, const Fr *msgs, size_t L) const;
};

inline void SecretKey::sign(Signature& sig, const PublicKey& pub, const Fr *msgs, size_t L) const
{
	if (L > s_maxMsgSize) {
		throw cybozu::Exception("too large L") << L;
	}
	Fr dom;
	local::calcDom(dom, pub.get_v(), L);
	local::Hash hash;
	hash << v << dom;
	for (size_t i = 0; i < L; i++) {
		hash << msgs[i];
	}
	Fr t;
	hash.get(t);
	Fr out[2];
	local::hash_to_scalar(out, t, 2);
	sig.e = out[0];
	sig.s = out[1];
	G1 B;
	local::calcB(B, sig.s, dom, msgs, L);
	Fr::add(t, v, sig.e);
	Fr::inv(t, t);
	G1::mul(sig.A, B, t);
}

inline bool Signature::verify(const PublicKey& pub, const Fr *msgs, size_t L) const
{
	if (L > s_maxMsgSize) return false;
	Fr dom;
	local::calcDom(dom, pub.get_v(), L);
	G1 B;
	local::calcB(B, s, dom, msgs, L);
	G1 v1[2] = { A, B };
	G2 v2[2] = { pub.get_v() + s_P2 * e, - s_P2 };
	GT out;
	millerLoopVec(out, v1, v2, 2);
	finalExp(out, out);
	return out.isOne();
}

class Proof {
	G1 Ap, Abar, D;
	Fr c, ehat, r2hat, r3hat, shat;
	Fr *mVec; // mVec must be U array of Fr
	uint32_t U; // U = msgN - discN
public:
};

// true if all discIdxs[i] < L
bool isValidDisc(size_t L, const uint32_t *discIdxs, size_t R)
{
	for (size_t i = 0; i < R; i++) {
		if (discIdxs[i] >= L) return false;
	}
	return true;
}

#if 0
/*
	L : number of all msgs
	R : number of disclosed msgs
	msgs[discIdxs[i]] : disclosed messages for i in [0, R)
*/
bool proofGen(Proof& prf, const PublicKey& pub, const Signature& sig, const Fr *msgs, size_t L, const uint32_t *discIdxs, size_t R)
{
	if (L > s_maxMsgSize) return false;
	if (L < R) return false;
	const size_t U = L - R;
	if (!isValidDisc(L, discIdxs, R)) return false;
	Fr dom;
	local::calcDom(dom, pub.get_v(), L);
	return true;
}
#endif

} } // mcl::bbs

