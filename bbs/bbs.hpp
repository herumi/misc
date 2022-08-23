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
void calcHash(Fr& out, const G2& pk, size_t n)
{
	Hash hash;
	hash << pk << n << s_Q1 << s_Q2;
	for (size_t i = 0; i < n; i++) {
		hash << s_H[i];
	}
	hash.get(out);
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
	friend class Signature;
public:
};

class Signature {
	G1 A;
	Fr e, s;
	friend class SecretKey;
public:
	bool verify(const PublicKey& pub, const Fr *msgVec, size_t n) const;
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
	void sign(Signature& sig, const PublicKey& pub, const Fr *msgVec, size_t n) const
	{
		if (n > s_maxMsgSize) {
			throw cybozu::Exception("too large n") << n;
		}
		Fr tt[3];
		Fr& e = tt[0];
		Fr& s = tt[1];
		Fr& dom = tt[2];
		local::calcHash(dom, pub.v, n);
	PUT(dom);
		Fr t;
		local::Hash hash;
		hash << v << dom;
		for (size_t i = 0; i < n; i++) {
			hash << msgVec[i];
		}
		hash.get(t);
		for (int i = 0; i < 2; i++) {
			local::Hash hash2;
			hash2 << t << i;
			hash2.get(tt[i]);
		}
		G1 B;
		G1::mulVec(B, &s_Q1, &s, 2); // B = s_Q1 * s + s_Q2 * dom
	PUT(B);
		B += s_P1;
		G1::mulVec(sig.A, s_H, msgVec, n); // A = sum s_H[i] * msgVec[i]
		B += sig.A;
		Fr::add(t, v, e);
		Fr::inv(t, t);
		G1::mul(sig.A, B, t);
		sig.e = e;
		sig.s = s;
	}
};

inline bool Signature::verify(const PublicKey& pub, const Fr *msgVec, size_t n) const
{
	if (n > s_maxMsgSize) {
		throw cybozu::Exception("too large n") << n;
	}
	Fr tt[2];
	tt[0] = s;
	local::calcHash(tt[1], pub.v, n);
PUT(tt[1]);
	G1 B;
	G1::mulVec(B, &s_Q1, &s, 2); // B = s_Q1 * s + s_Q2 * dom
PUT(B);
	B += s_P1;
	G1 T;
	G1::mulVec(T, s_H, msgVec, n); // T = sum s_H[i] * msgVec[i]
	B += T;
	G1 v1[2] = { A, B };
	G2 v2[2] = { pub.v + s_P2 * e, - s_P2 };
	GT out;
	millerLoopVec(out, v1, v2, 2);
	finalExp(out, out);
	return out.isOne();
}

} } // mcl::bbs

