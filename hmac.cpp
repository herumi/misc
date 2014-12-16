#include <stdio.h>
#include <openssl/hmac.h>
#include <cybozu/mmap.hpp>

int main()
{
	std::string out(21, 0);
	std::string key("\xC9\xFA\xCA\x54\x36\x84\x99\x06\xB6\x00\xDE\x95\xE1\x55\xB4\x7A\x01\xAB\xED\xD0", 20);
	cybozu::Mmap m("hmac.cpp");

	unsigned int outLen = 0;
	if (HMAC(EVP_sha1(), key.c_str(), key.size(), (const uint8_t*)m.get(), m.size(), (uint8_t*)&out[0], &outLen)) {
		out.resize(outLen);
		fwrite(out.c_str(), 1, outLen, stdout);
	}
}

