#pragma once
/**
	@file
	@brief TFNS
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause

	reference : https://doi.org/10.1587/transfun.2020CIP0010
	Strongly Secure Identity-Based Key Exchange with Single Pairing Operation
*/
#ifndef CYBOZU_DONT_USE_OPENSSL
	#define CYBOZU_DONT_USE_OPENSSL
#endif
#ifndef MCL_DONT_USE_OPENSSL
	#define MCL_DONT_USE_OPENSSL
#endif
#include <mcl/bls12_381.hpp>
#include <cybozu/sha2.hpp>
#include <cybozu/serializer.hpp>

namespace tfns {

using namespace mcl::bn;

static const size_t mdSize = 32;

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
	void digest(uint8_t md[mdSize])
	{
		h_.digest(md, mdSize, 0, 0);
	}
	template<class F>
	void get(F& x)
	{
		uint8_t md[mdSize];
		digest(md);
		x.setLittleEndianMod(md, mdSize);
		h_.clear();
	}
};

// reorder idA and idB lexicographically.
inline void make_d(Fr& d, const G1& epk, const std::string& idA, const std::string& idB)
{
	const std::string *pA = &idA;
	const std::string *pB = &idB;
	if (*pB > *pA) std::swap(pA, pB);
	Hash H;
	H << epk << *pA << *pB;
	H.get(d);
}

} // local

template<int dummy = 0>
struct TFNST {
	struct MainSecretKey;
	struct SecretKey;
	static G1 P_;
	static G2 Q_;

	struct SecretKey : G2 {
		void make_x(Fr& x, const Fr& esk) const
		{
			local::Hash H;
			H << *this << esk;
			H.get(x);
		}
		/*
			esk : rand
			x : H(sk, esk)
			epk : (H(id) P + mpk) * x
		*/
		void makeEPK(G1& epk, Fr& x, const std::string& id, const G1& mpk) const
		{
			local::Hash H;
			Fr h;
			H << id;
			H.get(h);
			G1::mul(epk, P_, h);
			epk += mpk;
			Fr esk;
			esk.setByCSPRNG();
			make_x(x, esk);
			epk *= x;
			epk.normalize();
		}
		/*
			I am userB and *this is skB
		*/
		void makeGT(GT& e, const Fr& xB, const Fr& dB, const std::string& idB, const std::string& idA, const G1& epkA, const G1& mpk) const
		{
			//recover dA from epkA, idA, idB
			Fr dA;
			local::make_d(dA, epkA, idA, idB);
			const G2& skB(*this);
			Fr hashedId;
			local::Hash H;
			H << idB;
			H.get(hashedId);
			G1 T;
			G1::mul(T, P_, hashedId);
			T += mpk;
			T *= dA;
			T += epkA; // epkA + dA(mpk + hashedId P)
			T *= dB + xB; // (epkA + dA(mpk + hashedId P))(dB + xB)
			pairing(e, T, skB);
		}
		void makeSessionKey(uint8_t md[32], const Fr& xB, const Fr& dB, const std::string& idB, const G1& epkB, const std::string& idA, const G1& epkA, const G1& mpk) const
		{
			GT e;
			makeGT(e, xB, dB, idB, idA, epkA, mpk);
			local::Hash H;
			const std::string *p_idA = &idA;
			const std::string *p_idB = &idB;
			const G1 *p_epkA = &epkA;
			const G1 *p_epkB = &epkB;
			// sort by idA, idB
			if (*p_idB > *p_idA) {
				std::swap(p_idA, p_idB);
				std::swap(p_epkA, p_epkB);
			}
			H << e << *p_idA << *p_idB << *p_epkA << *p_epkB;
			H.digest(md);
		}
	};

	struct MainSecretKey : Fr {
		// mpk = msk * P
		void getMainPublicKey(G1& mpk) const
		{
			G1::mul(mpk, P_, *this);
			mpk.normalize();
		}
		// sec = (1/(H(id) + msk)) Q
		void getSecretKey(SecretKey& sec, const std::string& id) const
		{
			Fr h;
			local::Hash H;
			H << id;
			H.get(h);
			h += *this;
			Fr::inv(h, h);
			SecretKey::mul(sec, Q_, h);
		}
	};
};

typedef TFNST<> TFNS;

template<int dummy> mcl::bn::G1 TFNST<dummy>::P_;
template<int dummy> mcl::bn::G2 TFNST<dummy>::Q_;

typedef TFNS::MainSecretKey MainSecretKey;
typedef G1 MainPublicKey;
typedef TFNS::SecretKey SecretKey;
typedef G1 PublicKey;
typedef G1 EphemeralPublicKey;
typedef mcl::bn::Fr Fr;
typedef uint8_t md[mdSize];

inline void init(const mcl::CurveParam& cp = mcl::BLS12_381)
{
	initPairing(cp);
	hashAndMapToG1(TFNS::P_, "0");
	hashAndMapToG2(TFNS::Q_, "0");
}
} // tfns

