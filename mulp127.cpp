#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include <stdint.h>
#include <mie/fp.hpp>
#include <mie/gmp_util.hpp>
#include <iostream>
#include <cybozu/benchmark.hpp>

typedef mie::FpT<mie::Gmp> Fp;
const int NN = 1000;
//#define USE_LOOP

/*
	p = 2^127 - 1
*/
struct F {
	uint64_t x[2];
	std::string toStr() const
	{
		char buf[64];
		snprintf(buf, sizeof(buf), "%llx%016llx", (long long)x[1], (long long)x[0]);
		return buf;
	}
};

std::ostream& operator<<(std::ostream& os, const F& x)
{
	return os << x.toStr();
}

F toF(const Fp& x)
{
	F y;
	y.x[0] = y.x[1] = 0;
	for (size_t i = 0; i < Fp::getBlockSize(x); i++) {
		y.x[i] = Fp::getBlock(x, i);
	}
	return y;
}

Fp toFp(const F& x)
{
	Fp y;
	y.setRaw(x.x, 2);
	return y;
}

// 21clk
void (*Fmul)(F& z, const F& x, const F& y);
// 19clk
void (*Fsqr)(F& z, const F& x);

struct Code : public Xbyak::CodeGenerator {
	Xbyak::util::Cpu cpu_;
	Code()
	{
		if (!cpu_.has(Xbyak::util::Cpu::tBMI2)) {
			fprintf(stderr, "mulx is not available\n");
			exit(1);
		}
		Fmul = getCurr<void (*)(F&, const F&, const F&)>();
		genMul();
		align(16);
		Fsqr = getCurr<void (*)(F&, const F&)>();
		genSqr();
	}
	void genMul()
	{
		// mulx(H, L, x) = [H:L] = x * rdx
		using namespace Xbyak;
		using namespace Xbyak::util;
#ifdef USE_LOOP
		StackFrame sf(this, 3, 8 | UseRDX | UseRCX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];

		const Reg64& t0 = sf.t[0];
		const Reg64& t1 = sf.t[1];
		const Reg64& t2 = sf.t[2];
		const Reg64& t3 = sf.t[3];
		const Reg64& t4 = sf.t[4];
		const Reg64& t5 = sf.t[5];
		const Reg64& t6 = sf.t[6];
		const Reg64& t7 = sf.t[7];
		/*
			x = [a:b]
			y = [c:d]
		*/
		mov(rcx, NN);
	L("@@");
		mov(rdx, ptr [px]); // b
		mulx(t1, t0, ptr [py]); // [t1:t0] = bd
		mulx(t3, t2, ptr [py + 8]); // [t3:t2] = bc
		mov(rdx, ptr [px + 8]); // a
		mulx(t5, t4, ptr [py]); // [t5:t4] = ad
		mulx(t7, t6, ptr [py + 8]); // [py:px] = ac
		xor_(eax, eax);
		add(t0, t6);
		adc(t1, t7);
		adc(rax, 0);
		add(t0, t5);
		adc(t1, t4);
		adc(rax, 0);
		add(t0, t3);
		adc(t1, t2);
		adc(rax, 0);
		add(t0, rax);
		adc(t1, 0);
		mov(ptr [pz], t0);
		mov(ptr [pz + 8], t1);
		dec(rcx);
		jnz("@b");
#else
		StackFrame sf(this, 3, 6 | UseRDX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];

		const Reg64& t0 = sf.t[0];
		const Reg64& t1 = sf.t[1];
		const Reg64& t2 = sf.t[2];
		const Reg64& t3 = sf.t[3];
		const Reg64& t4 = sf.t[4];
		const Reg64& t5 = sf.t[5];
		/*
			x = [a:b]
			y = [c:d]
		*/
		mov(rdx, ptr [px]); // b
		mulx(t1, t0, ptr [py]); // [t1:t0] = bd
		mulx(t3, t2, ptr [py + 8]); // [t3:t2] = bc
		mov(rdx, ptr [px + 8]); // a
		mulx(t5, t4, ptr [py]); // [t5:t4] = ad
		mulx(py, px, ptr [py + 8]); // [py:px] = ac
		xor_(eax, eax);
		add(t0, px);
		adc(t1, py);
		adc(rax, 0);
		add(t0, t5);
		adc(t1, t4);
		adc(rax, 0);
		add(t0, t3);
		adc(t1, t2);
		adc(rax, 0);
		add(t0, rax);
		adc(t1, 0);
		mov(ptr [pz], t0);
		mov(ptr [pz + 8], t1);
#endif
	}
	void genSqr()
	{
		// mulx(H, L, x) = [H:L] = x * rdx
		using namespace Xbyak;
		using namespace Xbyak::util;
#ifdef USE_LOOP
		StackFrame sf(this, 2, 6 | UseRDX | UseRCX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];

		const Reg64& t0 = sf.t[0];
		const Reg64& t1 = sf.t[1];
		const Reg64& t2 = sf.t[2];
		const Reg64& t3 = sf.t[3];
		const Reg64& t4 = sf.t[4];
		const Reg64& t5 = sf.t[5];
		/*
			x = [a:b]
		*/
		mov(rcx, NN);
	L("@@");
		mov(rdx, ptr [px]); // b
		mulx(t1, t0, rdx); // [t1:t0] = b^2
		mulx(t3, t2, ptr [px + 8]); // [t3:t2] = ab
		mov(rdx, ptr [px + 8]); // a
		mulx(t4, t5, rdx); // [t4:px] = a^2
		xor_(eax, eax);
		add(t0, t5);
		adc(t1, t4);
		adc(rax, 0);
		add(t0, t3);
		adc(t1, t2);
		adc(rax, 0);
		add(t0, t3);
		adc(t1, t2);
		adc(rax, 0);
		add(t0, rax);
		adc(t1, 0);
		mov(ptr [pz], t0);
		mov(ptr [pz + 8], t1);
		dec(rcx);
		jnz("@b");
#else
		StackFrame sf(this, 2, 5 | UseRDX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];

		const Reg64& t0 = sf.t[0];
		const Reg64& t1 = sf.t[1];
		const Reg64& t2 = sf.t[2];
		const Reg64& t3 = sf.t[3];
		const Reg64& t4 = sf.t[4];
		/*
			x = [a:b]
		*/
		mov(rdx, ptr [px]); // b
		mulx(t1, t0, rdx); // [t1:t0] = b^2
		mulx(t3, t2, ptr [px + 8]); // [t3:t2] = ab
		mov(rdx, ptr [px + 8]); // a
		mulx(t4, px, rdx); // [t4:px] = a^2
		xor_(eax, eax);
		add(t0, px);
		adc(t1, t4);
		adc(rax, 0);
		add(t0, t3);
		adc(t1, t2);
		adc(rax, 0);
		add(t0, t3);
		adc(t1, t2);
		adc(rax, 0);
		add(t0, rax);
		adc(t1, 0);
		mov(ptr [pz], t0);
		mov(ptr [pz + 8], t1);
#endif
	}
} s_code;

#define PUT(x) std::cout << #x"=" << x << std::endl
int main()
	try
{
	Fp::setModulo("0xffffffffffffffffffffffffffffffff");
	Fp x, y;
	F a, b;
	std::cout << std::hex;
	x.set("0x12345678901234567890123456789");
	y.set("0x23948202345678901234567893456789");
	a = toF(x);
	b = toF(y);
	for (int i = 0; i < 1000; i++) {
		x *= y;
		Fmul(a, a, b);
	}
	PUT(x);
	PUT(a);
	for (int i = 0; i < 1000; i++) {
		x *= x;
		Fmul(a, a, a);
	}
	PUT(x);
	PUT(a);
	CYBOZU_BENCH("Fmul", Fmul, a, a, b);
	CYBOZU_BENCH("Fsqr", Fsqr, a, a);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
