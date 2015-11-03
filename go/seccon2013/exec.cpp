#include <stdio.h>
#include <string>
#include <cybozu/atoi.hpp>
#include <cybozu/mmap.hpp>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>

struct A : Xbyak::CodeGenerator {
	A(const char *p, size_t n)
	{
		for (size_t i = 0; i < n; i += 2) {
			uint8_t v = cybozu::hextoi(p + i, 2);
			fprintf(stderr, "%02x ", v);
			db(v);
		}
		fprintf(stderr, "\n");
	}
};

int main(int argc, char *argv[])
	try
{
	if (argc != 3) {
		puts("exec line file number");
		return 1;
	}
	const std::string file = argv[1];
	const size_t n = atoi(argv[2]);

	cybozu::Mmap m(file);
	const char *p = m.get();
	size_t size = m.size();
	for (size_t i = 0; i <= n; i++) {
		const char *q = (const char*)memchr(p, ' ', size);
		if (q == NULL) return 1;
		size -= (q + 1) - p;
		p = q + 1;
	}
	A a(p, 62);
	a.getCode<void(*)()>()();
} catch (std::exception& e) {
	fprintf(stderr, "err %s\n", e.what());
	return 1;
}
