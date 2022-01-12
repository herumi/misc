#define PUT(x) std::cout << #x "=" << (x) << std::endl;
#include <cybozu/test.hpp>
#include <tfns.hpp>
#include <cybozu/xorshift.hpp>

CYBOZU_TEST_AUTO(correctness)
{
	cybozu::XorShift rg;
	mcl::fp::RandGen::setRandGen(rg);
	tfns::init();	
	const std::string idA = "alice";
	const std::string idB = "bob";
	tfns::MainSecretKey msk;
	tfns::MainPublicKey mpk;
	tfns::SecretKey skA;
	tfns::SecretKey skB;
	tfns::PublicKey pkA;
	tfns::PublicKey pkB;
	tfns::EphemeralPublicKey epkA;
	tfns::EphemeralPublicKey epkB;
	tfns::Fr xA;
	tfns::Fr xB;
//	tfns::md mdA;
//	tfns::md mdB;

	// main key generation
	msk.setByCSPRNG();
	msk.getMainPublicKey(mpk);

	// make secret key for each id
	msk.getSecretKey(skA, idA);
	msk.getSecretKey(skB, idB);

	// make ephemeral public key and values derived from esk
	skA.makeEPK(epkA, xA, idA, mpk);
	skB.makeEPK(epkB, xB, idB, mpk);

	using namespace mcl::bn;
	const auto& P = tfns::TFNS::P_;
	GT e;
	Fr dA, dB;
	tfns::local::make_d(dA, epkA, idA, idB);
	tfns::local::make_d(dB, epkB, idA, idB);
tfns::local::Hash H;
H << idB;
Fr iB;
H.get(iB);
G1 T1 = P * (msk + iB); // ok1
PUT(T1);
#if 0
G1 T2 = T1 * dA;
T2 += T1 * xA;
#else
T1 *= xA + dA; // ok
#endif
T1 *= xB + dB;
pairing(e, T1, skB);
	GT e1, e2;
	skB.makeGT(e1, mpk, idA, epkA, idB, epkB, xB);
//	skA.makeGT(e2, mpk, idB, epkB, idA, epkA, xA);

	CYBOZU_TEST_ASSERT(e == e1);
}
