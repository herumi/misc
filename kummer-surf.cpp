/*
	Haswell
	add/sub 0.25
	mov r,m 0.25
	mov m,r 0.5
	shlx    0.5
	bzhi    0.5
	adc/sbb 1
	shrd    1
	mulx    3

	smallModp
	0.5 + 0.5 + 0.25 + 1 = 2.25
	modp
	2 + 0.5 + 0.25 + 1 + smallModP = 6
	add
	0.25 + 1 + 2.25 = 3.5
	sub
	0.25 + 2 + 0.5 + 0.25 + 1 = 4
	M(mul)
	0.25 * 2 + 3 * 4 + 0.25 * 2 + 1 * 3 + 6 + 1 = 23
	S(sqrt)
	3 * 3 + 0.25 * 2 + 1 * 2 + 6 + 1 = 18.5
	m(mulC)
	3 * 2 + 0.25 * 2 + 1 * 3 + 0.5 + 2.25 = 12.25
	H
	add * 4 + sub * 4 + load * 8 + store * 4 = 14 + 16 + 4 + 4 = 38
	Fig23
	10 * M + 9 * S + 6 * m + 4 * H + load * 18 + store * 3 = 634
	634/1.66 = 381
	1.66 inst per cycle by perf stat
	real score 374
*/
#define XBYAK_NO_OP_NAMES
#include <stdint.h>
#include <mie/fp.hpp>
#include <mie/gmp_util.hpp>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include <iostream>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>

typedef mie::FpT<mie::Gmp> Fp;
cybozu::XorShift rg;

struct F;
void (*Fnormalize)(const F& x); // mod p in 2^127-1
void (*Fadd)(F& z, const F& x, const F& y);
void (*Fsub)(F& z, const F& x, const F& y);
void (*Fmul)(F& z, const F& x, const F& y);
void (*Fsqr)(F& z, const F& x);

/*
	p = 2^127 - 1
	[a:b] means = a * N + b where N = 2^64
	we use a little redudant representation, i.e.,
	0 <= [a:b] <= p (including p)
	the value of 'mod p' may be p
*/
struct F {
	mutable uint64_t x[2];
	std::string toStr() const
	{
		Fnormalize(*this);
		char buf[64];
		CYBOZU_SNPRINTF(buf, sizeof(buf), "%llx%016llx", (long long)x[1], (long long)x[0]);
		return buf;
	}
	void setRand()
	{
		x[0] = rg.get64() & (uint64_t(-1) >> 1);
		x[1] = rg.get64() & (uint64_t(-1) >> 1);
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

//  in = [x2, y2, z2, t2, x3, y3, z3, t3]
// sub = [x1/y1, x1/z2, x1/t1]
// out = [x4, y4, z4, t4, x5, y5, z5, t5]
void (*Fig23)(F out[8], const F in[8], const F sub[3]);

struct Code : public Xbyak::CodeGenerator {
	typedef Xbyak::RegExp RegExp;
	typedef Xbyak::Reg64 Reg64;
	typedef Xbyak::Operand Operand;
	typedef Xbyak::util::StackFrame StackFrame;
	typedef Xbyak::util::Pack Pack;
	static const int UseRDX = Xbyak::util::UseRDX;
	static const int UseRCX = Xbyak::util::UseRCX;
	Xbyak::util::Cpu cpu_;
	Code()
		: Xbyak::CodeGenerator(8192)
	{
		if (!cpu_.has(Xbyak::util::Cpu::tBMI2)) {
			fprintf(stderr, "mulx is not available\n");
			exit(1);
		}
		align(16);
		Fadd = getCurr<void (*)(F&, const F&, const F&)>();
		genFadd();

		align(16);
		Fsub = getCurr<void (*)(F&, const F&, const F&)>();
		genFsub();

		align(16);
		Fmul = getCurr<void (*)(F&, const F&, const F&)>();
		genFmul();

		align(16);
		Fsqr = getCurr<void (*)(F&, const F&)>();
		genFsqr();

		align(16);
		Fig23 = getCurr<void (*)(F*, const F*, const F*)>();
		genFig23();

		align(16);
		Fnormalize = getCurr<void (*)(const F&)>();
		genFnormalize();
	}
	/*
		[yH:yL] *= px[]
		destroy : t[0..4], rax, rdx
	*/
	void genMul(const RegExp& pz, const RegExp& px, const Reg64& yH, const Reg64& yL, const Pack& t)
	{
		assert(t.size() >= 4);
		/*
			x = [a:b]
			y = [c:d]
		*/
		mov(rdx, ptr [px]); // b
		mulx(t[1], t[0], yL); // [t1:t0] = bd
		mulx(t[3], t[2], yH); // [t3:t2] = bc
		mov(rdx, ptr [px + 8]); // a
		mulx(yL, rax, yL); // [yL:rax] = ad
		mulx(yH, rdx, yH); // [yH:rdx] = ac
		/*
			|a c|b d|
			  |a d|
			  |b c|
			x = aN + b
			y = cN + d
			N = 2^64
			ac <= (N/2 - 1)(N/2 - 1)
			ad + bc <= (N - 2)(N - 1)
			[yH:rdx][t1:t0]
			    [yL:rax]
			    [t3:t2]
		*/
		add(rax, t[2]);
		adc(yL, t[3]);
		add(t[1], rax);
		adc(yL, rdx);
		adc(yH, 0); // [yH:yL:t1:t0] = (aN+b)(cN+d)
		genModp(yH, yL, t[1], t[0]);
		store(pz, yH, yL);
	}
	/*
		[t1:t0] = [xH:xL]^2
		assume x in [0, p]
	*/
	void genSqr(const RegExp& pz, const Reg64& xH, const Reg64& xL, const Pack& t)
	{
		assert(t.size() >= 3);
		/*
			x = [a:b]
			a <= 2^63 - 1
		*/
		mov(rdx, xH); // a
		mulx(t[1], t[0], rdx); // [t1:t0] = a^2
		add(rdx, rdx);
		mulx(xH, t[2], xL); // [xH:t2] = 2ab
		mov(rdx, xL); // b
		mulx(xL, rdx, rdx); // [xL:rdx] = b^2
		/*
			[t1:t0][xL:rdx]
			    [xH:t2]
		*/
		add(t[2], xL);
		adc(t[0], xH);
		adc(t[1], 0);
		genModp(t[1], t[0], t[2], rdx);
		store(pz, t[1], t[0]);
	}
	/*
		[xH:xL] *= c
	*/
	void genMulC(const Reg64& xH, const Reg64& xL,const uint64_t c,  const Reg64& t)
	{
		mov(rdx, c);
		mulx(t, xL, xL); // [t:xL] = xL * c
		mulx(rdx, xH, xH); // [rdx:xH] = xH * c
		add(xH, t);
		adc(rdx, 0); // [rdx:xH:xL]
		shld(rdx, xH, 1);
		mov(eax, 63);
		bzhi(xH, xH, rax);
		add(xL, rdx);
		adc(xH, 0); // [xH:xL]
		genSmallModp(xH, xL, rdx, rax);
	}
	void genFmul()
	{
		// mulx(H, L, x) = [H:L] = x * rdx
		StackFrame sf(this, 3, 6 | UseRDX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& yH = sf.p[2];
		const Reg64& yL = sf.t[0];
		const Pack t = sf.t.sub(1);
		mov(yL, ptr [yH]);
		mov(yH, ptr [yH + 8]);
		genMul(pz, px, yH, yL, t);
	}
	void genFadd()
	{
		StackFrame sf(this, 3, 2);
		const Reg64& pz = sf.p[0];
		const Reg64& xH = sf.p[1];
		const Reg64& yH = sf.p[2];
		const Reg64& xL = sf.t[0];
		const Reg64& yL = sf.t[1];
		load(xH, xL, xH);
		load(yH, yL, yH);
		genAdd(xH, xL, yH, yL);
		store(pz, xH, xL);
	}
	void genFsub()
	{
		StackFrame sf(this, 3, 2);
		const Reg64& pz = sf.p[0];
		const Reg64& xH = sf.p[1];
		const Reg64& yH = sf.p[2];
		const Reg64& xL = sf.t[0];
		const Reg64& yL = sf.t[1];
		load(xH, xL, xH);
		load(yH, yL, yH);
		genSub(xH, xL, yH, yL);
		store(pz, xH, xL);
	}
	/*
		[H:L] -= p if [H:L] > p
		@input [H:L]
		@note [H:L] does not change if [H:L] == p
		assume i63 = 63
		destroy t
	*/
	void genSmallModp(const Reg64& H, const Reg64& L, const Reg64& t, const Reg64& i63)
	{
		shrx(t, H, i63); // t = (H == 2^63) ? 1 : 0
		bzhi(H, H, i63); // H &= (1 << 63) - 1
		add(L, t);
		adc(H, 0);
	}
	/*
		[t3:t2] = [t3:t2:t1:t0] mod p
		destroy rax
		assume t3 <= 2^62
	*/
	void genModp(const Reg64& t3, const Reg64& t2, const Reg64& t1, const Reg64& t0)
	{
		shld(t3, t2, 1); // t3 = [t3:t2] <<= 1
		shld(t2, t1, 1); // t2 = [t2:t1] <<= 1
		mov(eax, 63);
		bzhi(t1, t1, rax);
		add(t2, t0);
		adc(t3, t1);
		// [t3:t2]
		genSmallModp(t3, t2, t0, rax);
	}
	/*
		[xH:xL] += [yH:yL]
		assume x, y in [0, p]
		output [0, p]
		destroy yL, rax
	*/
	void genAdd(const Reg64& xH, const Reg64& xL, const Reg64& yH, const Reg64& yL)
	{
		add(xL, yL);
		adc(xH, yH);
		mov(eax, 63);
		genSmallModp(xH, xL, yL, rax);
	}
	/*
		[xH:xL] -= [yH:yL]
		assume x, y in [0, p]
		output [0, p]
		destroy yL, rax
	*/
	void genSub(const Reg64& xH, const Reg64& xL, const Reg64& yH, const Reg64& yL)
	{
		sub(xL, yL);
		sbb(xH, yH);
		mov(eax, 63);
		shrx(yL, xH, rax); // rax = x < y ? 1 : 0
		sub(xL, yL);
		sbb(xH, 0);
		bzhi(xH, xH, rax);
	}
	void genFnormalize()
	{
		StackFrame sf(this, 1, 3);
		const Reg64& pz = sf.p[0];
		const Reg64& zL = sf.t[0];
		const Reg64& zH = sf.t[1];
		const Reg64& t = sf.t[2];
		load(zH, zL, pz);
		mov(eax, 63);
		genSmallModp(zH, zL, t, rax);
		store(pz, zH, zL);
	}
	void genFsqr()
	{
		// mulx(H, L, x) = [H:L] = x * rdx
		StackFrame sf(this, 2, 6 | UseRDX);
		const Reg64& pz = sf.p[0];
		const Reg64& xH = sf.p[1];
		const Reg64& xL = sf.t[0];
		const Pack t = sf.t.sub(1);
		mov(xL, ptr [xH]);
		mov(xH, ptr [xH + 8]);
		genSqr(pz, xH, xL, t);
	}
	void load(const Reg64& H, const Reg64& L, const RegExp& m)
	{
		mov(L, ptr [m]);
		mov(H, ptr [m + 8]);
	}
	void store(const RegExp& m, const Reg64& H, const Reg64& L)
	{
		mov(ptr [m], L);
		mov(ptr [m + 8], H);
	}
	/*
		in : in = [t:z:y:x]
		out : out = [(x-y)-(z-t):(x-y)+(z-t):(x+y)-(z+t):(x+y)+(z+t)]
		destroy : tH, tL, t[0..5]
		allow out == in
	*/
	void genH(const RegExp& out, const RegExp& in, const Reg64& tH, const Reg64& tL, const Pack& t)
	{
		assert(t.size() >= 4);
		const int _x = 0 * 16;
		const int _y = 1 * 16;
		const int _z = 2 * 16;
		const int _t = 3 * 16;
		load(tH, tL, in + _x); // [tH:tL] = x
		load(t[1], t[0], in + _y);
		genAdd(t[1], t[0], tH, tL); // t[1:0] = x+y
		load(tH, tL, in + _z);
		load(t[3], t[2], in + _t);
		genAdd(t[3], t[2], tH, tL); // t[3:2] = z+t
		mov(tL, t[0]);
		mov(tH, t[1]); // t[5:4] = x+y
		genAdd(t[1], t[0], t[3], t[2]); // (x+y) + (z+t)
		genSub(tH, tL, t[3], t[2]); // (x+y) - (z+t)
		load(t[3], t[2], in + _x);
		store(out + _x, t[1], t[0]); // (x+y) + (z+t)
		load(t[1], t[0], in + _y);
		store(out + _y, tH, tL); // (x+y) - (z+t)
		load(tH, tL, in + _t);
		genSub(t[3], t[2], t[1], t[0]); // x-y
		load(t[1], t[0], in + _z);
		genSub(t[1], t[0], tH, tL); // z-t
		mov(tL, t[2]);
		mov(tH, t[3]); // t[3:2] = x-y
		genAdd(t[3], t[2], t[1], t[0]); // (x-y)+(z-t)
		genSub(tH, tL, t[1], t[0]); // (x-y)-(z-t)
		store(out + _z, t[3], t[2]);
		store(out + _t, tH, tL);
	}
	void genFig23()
	{
		StackFrame sf(this, 3, 8 | UseRDX, 8 * 2 * 8);
		const Reg64& out = sf.p[0];
		const Reg64& in = sf.p[1];
		const Reg64& sub = sf.p[2];
		const Reg64& tL = sf.t[0];
		const Reg64& tH = sf.t[1];
		const Reg64& cL = sf.t[2];
		const Reg64& cH = sf.t[3];
		const Pack t = sf.t.sub(4);
		assert(t.size() >= 4);

		// H
		genH(rsp, in, tH, tL, t);
		genH(rsp + 4 * 16, in + 4 * 16, tH, tL, t);
		// use correct constant for A2/B2, A2/C2, etc
		// now random value
		const int cTbl[6] = { 3, 33, 123, 999, 1234, 5432 };

		// y2' = y2 * A2/B2, z2' = z2 * A2/C2, t2' = t2 * A2/D2
		for (int i = 0; i < 3; i++) {
			load(cH, cL, rsp + (i + 1) * 16);
			genMulC(cH, cL, cTbl[i], t[0]); // y2' = y2 * A2/B2
			mov(tL, cL);
			mov(tH, cH);
			genMul(rsp + (i + 5) * 16, rsp + (i + 5) * 16, tH, tL, t); // y3 * y2'
			genMul(rsp + (i + 1) * 16, rsp + (i + 1) * 16, cH, cL, t); // y2 * y2'
		}
		load(cH, cL, rsp);
		mov(tL, cL);
		mov(tH, cH);
		genMul(rsp + 4 * 16, rsp + 4 * 16, tH, tL, t); // x3 * x2
		genSqr(rsp, cH, cL, t); // x2^2

		genH(rsp, rsp, tH, tL, t);
		genH(rsp + 4 * 16, rsp + 4 * 16, tH, tL, t);

		for (int i = 0; i < 8; i++) {
			load(tH, tL, rsp + i * 16);
			if (i == 0 || i == 4) {
				genSqr(out + i * 16, tH, tL, t);
			} else {
				genSqr(rsp + i * 16, tH, tL, t);
			}
		}
		for (int i = 0; i < 3; i++) {
			load(tH, tL, rsp + (i + 1) * 16);
			genMulC(tH, tL, cTbl[i + 3], t[0]);
			store(out + (i + 1) * 16, tH, tL);
		}
		for (int i = 0; i < 3; i++) {
			load(tH, tL, rsp + (i + 5) * 16);
			genMul(out + (i + 5) * 16, sub + i * 16, tH, tL, t);
		}
	}
} s_code;

#define PUT(x) std::cout << #x"=" << x << std::endl
int main()
	try
{
	Fp::setModulo("0x7fffffffffffffffffffffffffffffff");
	Fp x, y;
	F a, b;
	std::cout << std::hex;
	x.set("0x79912345678901234567890123456789");
	y.set("0x23948202345678901234567893456789");
	a = toF(x);
	b = toF(y);
	puts("add");
	for (int i = 0; i < 1000; i++) {
		x += x;
		Fadd(a, a, a);
	}
	PUT(x);
	PUT(a);
	puts("sub");
	for (int i = 0; i < 1000; i++) {
		y -= x;
		Fsub(b, b, a);
	}
	PUT(y);
	PUT(b);
	puts("mul");
	for (int i = 0; i < 1000; i++) {
		x *= y;
		Fmul(a, a, b);
	}
	PUT(x);
	PUT(a);
	puts("sqr");
	for (int i = 0; i < 1000; i++) {
		x *= x;
		Fsqr(a, a);
	}
	PUT(x);
	PUT(a);
	CYBOZU_BENCH("Fadd", Fadd, a, a, b);
	CYBOZU_BENCH("Fsub", Fadd, b, b, a);
	CYBOZU_BENCH("Fmul", Fmul, a, a, b);
	CYBOZU_BENCH("Fsqr", Fsqr, a, a);
	F in[8], sub[3];
	for (int i = 0; i < 8; i++) {
		in[i].setRand();
	}
	for (int i = 0; i < 3; i++) {
		sub[i].setRand();
	}
	CYBOZU_BENCH_C("Fig23", 10000, Fig23, in, in, sub);
	for (int i = 0; i < 8; i++) {
		printf("in[%d]=%s\n", i, in[i].toStr().c_str());
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
