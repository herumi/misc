#include "bbs.hpp"
#include <cybozu/test.hpp>

const size_t maxMsgSize = 32;

using namespace mcl::bbs;

CYBOZU_TEST_AUTO(init)
{
	init(maxMsgSize);
}

CYBOZU_TEST_AUTO(sign_verify)
{
	SecretKey sec;
	sec.initForDebug(123);
	PublicKey pub;
	sec.getPublicKey(pub);
	Fr msg;
	msg = 0x12345678;
	Signature sig;
	sig.sign(sec, pub, &msg, 1);
	CYBOZU_TEST_ASSERT(sig.verify(pub, &msg, 1));
	msg -= 1;
	CYBOZU_TEST_ASSERT(!sig.verify(pub, &msg, 1));
}

