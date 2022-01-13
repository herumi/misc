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

inline void make_d(Fr& d, const G1& epk, const std::string& idA, const std::string& idB)
{
	Hash H;
	H << epk << idA << idB;
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
		}
		/*
			I am userB and *this is skB
		*/
		void makeGT(GT& e, const G1& mpk, const std::string& idA, const G1& epkA, const std::string& idB, const G1& epkB, const Fr& x, bool IamB, const Fr& dA, const Fr& dB) const
		{
			const G2& skB(*this);
			Fr hashedId;
			local::Hash H;
			H << (IamB ? idB : idA);
			H.get(hashedId);
			G1 T;
			G1::mul(T, P_, hashedId);
			T += mpk;
			if (IamB) {
				T *= dA;
				T += epkA; // epkA + dA(mpk + hashedId P)
				T *= dB + x; // (epkA + dA(mpk + hashedId P))(dB + x)
			} else {
				T *= dB;
				T += epkB;
				T *= dA + x;
			}
			pairing(e, T, skB);
		}
		void makeSessionKey(uint8_t md[32], const G1& mpk, const std::string& idA, const G1& epkA, const std::string& idB, const G1& epkB, const Fr& xB) const
		{
			GT e;
			makeGT(e, mpk, idA, epkA, idB, epkB, xB);
			local::Hash H;
			H << e << idA << idB << epkA << epkB;
			H.digest(md);
		}
	};

	struct MainSecretKey : Fr {
		// mpk = msk * P
		void getMainPublicKey(G1& mpk) const
		{
			G1::mul(mpk, P_, *this);
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

