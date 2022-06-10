#include <cybozu/option.hpp>
#include <cybozu/itoa.hpp>
#include <xbyak/xbyak_util.h>

using namespace Xbyak;
using namespace Xbyak::util;

struct Param {
	size_t maxBitSize;
	void put() const
	{
		fprintf(stderr, "maxBitSize=%zd\n", maxBitSize);
	}
};

void hexdump(const uint8_t *begin, const uint8_t *end)
{
	size_t pos = 0;
	while (begin != end) {
		if (pos == 0) {
			printf("db ");
		}
		printf("0x%02x,", *begin);
		begin++;
		pos++;
		if (pos == 16) {
			printf("\n");
			pos = 0;
		}
	}
	printf("\n");
}

struct Code : Xbyak::CodeGenerator {
	const Param p;
	const size_t maxN;
	struct FuncProc {
		Code *c;
		const uint8_t *begin;
		FuncProc(Code *c, std::string name, size_t N) : c(c)
		{
			begin = c->getCurr();
			name += cybozu::itoa(N);
			const char *s = name.c_str();
			printf("align 16\n");
			printf("global %s\nglobal _%s\n%s:\n_%s:\n", s, s, s, s);
		}
		~FuncProc()
		{
			hexdump(begin, c->getCurr());
		}
	};
	Code(const Param& p)
		: Xbyak::CodeGenerator(4096, Xbyak::DontSetProtectRWE)
		, p(p)
		, maxN((p.maxBitSize + 63) / 64)
	{
		printf("segment .text\n");
		for (size_t i = 0; i <= maxN; i++) {
			FuncProc fp(this, "mclb_add", i);
			gen_add(i);
		}
	}
	void gen_add(size_t N)
	{
		StackFrame sf(this, 3);
		const Reg64& z = sf.p[0];
		const Reg64& x = sf.p[1];
		const Reg64& y = sf.p[2];
		for (size_t i = 0; i < N; i++) {
			mov(rax, ptr[x + i * 8]);
			if (i == 0) {
				add(rax, ptr[y + i * 8]);
			} else {
				adc(rax, ptr[y + i * 8]);
			}
			mov(ptr[z + i * 8], rax);
		}
		if (N == 0) {
			xor_(eax, eax);
		} else {
			setc(al);
			movzx(eax, al);
		}
	}
	void gen_sub(size_t N)
	{
		StackFrame sf(this, 3);
		const Reg64& z = sf.p[0];
		const Reg64& x = sf.p[1];
		const Reg64& y = sf.p[2];
		for (size_t i = 0; i < N; i++) {
			mov(rax, ptr[x + i * 8]);
			if (i == 0) {
				sub(rax, ptr[y + i * 8]);
			} else {
				sbb(rax, ptr[y + i * 8]);
			}
			mov(ptr[z + i * 8], rax);
		}
		if (N == 0) {
			xor_(eax, eax);
		} else {
			setc(al);
			movzx(eax, al);
		}
	}
};

int main(int argc, char *argv[])
{
	Param p;
	cybozu::Option opt;
	opt.appendOpt(&p.maxBitSize, 256/*512+32*/, "max", ": maxBitSize");
	opt.appendHelp("h", ": show this message");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	p.put();
	Code c(p);
}
