#define PUT(x) std::cout << #x "=" << x << std::endl;
#include <tfns.hpp>
#include <cybozu/test.hpp>
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
	PUT(msk);
	msk.getMainPublicKey(mpk);
	PUT(mpk);

	// make secret key for each id
	msk.getSecretKey(skA, idA);
	msk.getSecretKey(skB, idB);
	PUT(skA);
	PUT(skB);

	// make ephemeral public key and values derived from esk
	skA.makeEPK(epkA, xA, idA, mpk);
	PUT(epkA);
	PUT(xA);
	skB.makeEPK(epkB, xB, idB, mpk);
	PUT(epkB);
	PUT(xB);
puts("---");

	skB.makeSessionKey(mdB, mpk, idA, epkA, idB, epkB, xB, 1);
puts("---");
	skA.makeSessionKey(mdA, mpk, idA, epkA, idB, epkB, xA, 0);

//	CYBOZU_TEST_EQUAL_ARRAY(mdA, mdB, sizeof(mdA));
	CYBOZU_TEST_ASSERT(memcmp(mdA, mdB, sizeof(mdA)) == 0);
}
