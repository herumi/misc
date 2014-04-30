/*
	length-extension attack sample for SHA-1
*/
#include "sha1-len-ext-attack.hpp"
#include <stdio.h>
#include <string>

void put(const std::string& msg, bool allHex = true)
{
	for (size_t i = 0; i < msg.size(); i++) {
		const char c = msg[i];
		if (allHex) {
			printf("%02x", (unsigned char)msg[i]);
		} else {
			if (' ' <= c && c <= '~') {
				putchar(c);
			} else {
				printf("\\x%02x", (unsigned char)msg[i]);
			}
		}
	}
	printf("\n");
}

/*
	weak HMAC
	return sha1(<private key> || <message>)
*/
std::string weakHMAC(const std::string& s, const std::string& m)
{
	Sha1 sha1;
	return sha1.digest(s + m);
}

/*
	make string to pad m
	the size of m + padding(m) is 64 byte
*/

std::string padding(size_t len)
{
	if (len > 64 - 9) {
		printf("too long %d\n", (int)len);
		exit(1);
	}
	std::string pad;
	const size_t padSize = 64 - len;
	pad.resize(padSize);
	pad[0] = '\x80';
	/*
		set (len * 8) to last 8 byte of pad big endian
	*/
	len *= 8;
	pad[padSize - 2] = char(len >> 8);
	pad[padSize - 1] = char(len);
	return pad;
}

struct Pair {
	std::string msg;
	std::string mac;
	void validate(const std::string& s) const
	{
		const std::string m = weakHMAC(s, msg);
		if (m == mac) {
			printf("validation ok ");
			put(m);
		} else {
			printf("validation ng\n");
			printf("ok="); put(mac);
			printf("ng="); put(m);
		}
	}
};

/*
	make a new pair (msg2, mac2) such that mac2 = weakHMAC(s, msg2)
	where msg2 = <original msg> + <padding> + <any small string>

	@param sLen : size of s(the value of s is unknown)
	@param m, mac : correct pair such that mac = weakHMAC(s, mac)
	@param str : any small string
*/
Pair makeFakePair(size_t sLen, const std::string& msg, const std::string& mac, const std::string& str)
{
	if (str.size() >= 64 - 9) {
		printf("str is too long %d\n", (int)str.size());
		exit(1);
	}
	Pair pair;
	pair.msg = msg + padding(sLen + msg.size()) + str;

	Sha1 sha1;
	sha1.set(mac, 64); // recover the last state for mac
	pair.mac = sha1.digest(&str[0], str.size());
	return pair;
}

int main()
{
	/*
		s is a private key
	*/
	const std::string s = "7d64d66c09d9aea8";
	/*
		m is a message
	*/
	const std::string msg = "guest";

	const std::string mac = weakHMAC(s, msg);
	printf("msg=%s\n", msg.c_str());
	printf("weakHMAC=");
	put(mac);

	/*
		any string
	*/
	puts("attacker");
	printf("s size=%d\n", (int)s.size());
	const Pair pair = makeFakePair(s.size(), msg, mac, "xyz");
	printf("new msg=");
	put(pair.msg, false);

	puts("server");
	pair.validate(s);
}
