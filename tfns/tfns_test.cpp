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
	tfns::md mdA;
	tfns::md mdB;

	// main key generation
	msk.setByCSPRNG();
	msk.getMainPublicKey(mpk);

	// make secret key for each id
	msk.getSecretKey(skA, idA);
	msk.getSecretKey(skB, idB);

	// make ephemeral public key and values derived from esk
	skA.makeEPK(epkA, xA, idA, mpk);
	skB.makeEPK(epkB, xB, idB, mpk);

	skB.makeSessionKey(mdB, mpk, idA, epkA, idB, epkB, xB);
	skA.makeSessionKey(mdA, mpk, idB, epkB, idA, epkA, xA);

	CYBOZU_TEST_EQUAL_ARRAY(mdA, mdB, sizeof(mdA));
}
