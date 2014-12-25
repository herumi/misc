#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>

void (*f1)(uint64_t*, const uint64_t*, const uint64_t*);
void (*f2)(uint64_t*, const uint64_t*, const uint64_t*);
void (*f3)(uint64_t*, const uint64_t*, const uint64_t*);

const int S = 9;

using namespace Xbyak::util;
using namespace Xbyak;

struct Code : Xbyak::CodeGenerator {
	Code()
		try
	{
		f1 = getCurr<void (*)(uint64_t*, const uint64_t*, const uint64_t*)>();
		gen(1);

		align(16);
		f2 = getCurr<void (*)(uint64_t*, const uint64_t*, const uint64_t*)>();
		gen(2);

		align(16);
		f3 = getCurr<void (*)(uint64_t*, const uint64_t*, const uint64_t*)>();
		gen(3);
	} catch (std::exception& e) {
		printf("err %s\n", e.what());
		exit(1);
	}
	void movS(const RegExp& pz, const RegExp& px, const Reg64& t)
	{
		for (int i = 0; i < S; i++) {
			mov(t, ptr [px + i * 8]);
			mov(ptr [pz + i * 8], t);
		}
	}
	void subS(const RegExp& pz, const RegExp& px, const RegExp& py, const Reg64& t)
	{
		for (int i = 0; i < S; i++) {
			mov(t, ptr [px + i * 8]);
			if (i == 0) {
				sub(t, ptr [py + i * 8]);
			} else {
				sbb(t, ptr [py + i * 8]);
			}
			mov(ptr [pz + i * 8], t);
		}
	}
	void gen(int mode)
	{
		Xbyak::util::StackFrame sf(this, 3, 2, S * 8);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		const Reg64& t = rax;
		switch (mode) {
		case 1:
			inLocalLabel();
			subS(rsp, px, py, t);
			jc(".x_lt_y", T_NEAR);
			movS(pz, rsp, t);
			jmp(".exit", T_NEAR);
		L(".x_lt_y");
			subS(pz, py, px, t);
		L(".exit");
			outLocalLabel();
			break;
		case 2:
			inLocalLabel();
			for (int i = S - 1; i >= 0; i--) {
				mov(t, ptr [px + i * 8]);
				cmp(t, ptr [py + i * 8]);
				ja(".x_gt_y", T_NEAR);
				jb(".x_lt_y", T_NEAR);
			}
		L(".x_lt_y");
			subS(pz, py, px, t);
			jmp(".exit", T_NEAR);
		L(".x_gt_y");
			subS(pz, px, py, t);
		L(".exit");
			outLocalLabel();
			break;
		case 3:
			inLocalLabel();
			outLocalLabel();
			break;
		default:
			printf("bad mode=%d\n", mode);
			exit(1);
		}
	}
} s_code;

template<class F>
void test(const char *msg, F& f)
{
	const int N = 100000;
	uint64_t x[S] = {};
	uint64_t y[S] = {};
	cybozu::XorShift rg;
	for (int i = 0; i < S; i++) {
		x[i] = rg.get64();
		y[i] = rg.get64();
	}
	CYBOZU_BENCH_C(msg, N, x[S - 1] = rg.get64(); f, x, x, y);
	printf("%lld\n", (long long)x[S-1]);
}

int main()
{
	test("f1", f1);
	test("f2", f2);
}
