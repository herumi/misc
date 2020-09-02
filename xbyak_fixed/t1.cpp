#include <xbyak/xbyak_util.h>
#include <cybozu/stream.hpp>
#include <cybozu/itoa.hpp>
#include <fstream>

using namespace Xbyak;
using namespace Xbyak::util;

template<class OutputStream>
void put(OutputStream& os, const void *src, size_t n)
{
	const uint8_t *p = (const uint8_t*)src;
	while (n > 0) {
		size_t remain = (n >= 16) ? 16 : n;
		os.write("db ", 3);
		for (size_t i = 0; i < remain; i++) {
			char buf[] = "0x__,";
			cybozu::itohex(buf + 2, 2, *p++);
			os.write(buf, 5);
		}
		os.write("\n", 1);
		n -= remain;
	}
}

struct Code : CodeGenerator {
	Code()
	{
		StackFrame sf(this, 2);
		mov(rax, sf.p[0]);
		add(rax, sf.p[1]);
	}
};

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "t1 out-file\n");
		return 1;
	}
	std::ofstream ofs(argv[1], std::ios::binary);
	Code c;

	std::string header =
		"segment .text\n"
		"global fff:\n"
		"fff:\n";
	ofs.write(header.c_str(), header.size());
	put(ofs, c.getCode(), c.getSize());
}
