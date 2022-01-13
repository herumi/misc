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
	tfns::Fr xA, dA;
	tfns::Fr xB, dB;
	tfns::md mdA;
	tfns::md mdB;

	// main key generation
	msk.setByCSPRNG();
	msk.getMainPublicKey(mpk);

	// make secret key for each id
	msk.getSecretKey(skA, idA);
	msk.getSecretKey(skB, idB);

	// make ephemeral public key and values derived from esk
	// epkA, dA : public
	// skA, xA : secret
	skA.makeEPK(epkA, xA, idB, mpk);
	skB.makeEPK(epkB, xB, idA, mpk);

	{
		using namespace mcl::bn;
		GT e, e1, e2;
		const auto& P = tfns::TFNS::P_;
		const auto& Q = tfns::TFNS::Q_;
		tfns::local::make_d(dA, epkA, idA, idB);
		tfns::local::make_d(dB, epkB, idA, idB);
		// expected e
		pairing(e, P * (xA + dA) * (xB + dB), Q);
		skB.makeGT(e1, xB, dB, idB, idA, epkA, mpk);
		skA.makeGT(e2, xA, dA, idA, idB, epkB, mpk);

		CYBOZU_TEST_ASSERT(e == e1);
		CYBOZU_TEST_ASSERT(e == e2);
	}

	// user A sends epkA to B
	// user B has xB and dB
	skB.makeSessionKey(mdB, xB, dB, idB, epkB, idA, epkA, mpk);
	// user B sends epkB to B
	// user A has xA and dA
	skA.makeSessionKey(mdA, xA, dA, idA, epkA, idB, epkB, mpk);
	CYBOZU_TEST_EQUAL_ARRAY(mdA, mdB, sizeof(mdA));
}
