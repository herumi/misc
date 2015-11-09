#include <stdio.h>
#include <string>
#include <cybozu/atoi.hpp>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>

struct A : Xbyak::CodeGenerator {
	explicit A(const std::string& s)
	{
		for (size_t i = 0; i < s.size(); i += 2) {
			uint8_t v = cybozu::hextoi(s.c_str() + i, 2);
//			fprintf(stderr, "%02x ", v);
			db(v);
		}
//		fprintf(stderr, "\n");
	}
};

int main(int argc, char *argv[])
	try
{
	if (argc != 2) {
		puts("exec hexbinary");
		return 1;
	}
	const std::string s = argv[1];
	A a(s);
	a.getCode<void(*)()>()();
} catch (std::exception& e) {
	fprintf(stderr, "err %s\n", e.what());
	return 1;
}
