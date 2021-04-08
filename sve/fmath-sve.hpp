#pragma once
/**
	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <xbyak_aarch64/xbyak_aarch64.h>
#include <cmath>
#include <vector>

namespace fmath {

namespace local {

union fi {
	float f;
	uint32_t i;
};

inline float cvt(uint32_t x)
{
	fi fi;
	fi.i = x;
	return fi.f;
}

/*
f:=x->1+B*x+C*x^2;
g:=int((f(x)-2^x)^2,x=0..L);
sols:=solve({diff(g,B)=0,diff(g,C)=0},{B,C});
Digits:=100;
s:=eval(sols,L=0.015625);
evalf(s,30);
coeff1 = B, coeff2 = C
*/
struct ConstVar {
	float log2_e; // 1 / log2
	float one;
	float coeff1; // about log2
	float coeff2; // about 0.5 * log2 * log2
	uint32_t expMax;
	uint32_t expMin;
	static const uint32_t not_mask17= ~((1u << 17) - 1);
	float tanhRange;
	float f1p3;
	//
	void init()
	{
		log2_e = 1.0f / std::log(2.0f);
		one = 1.0f;
		coeff1 = 0.6931473921;
		coeff2 = 0.2413862043;
		expMax = 0x42b17218;
		expMin = 0xc47a0000;//0xc2aeac50;
		tanhRange = 0.01;
		f1p3 = 1.0f / 3;
	}
};

struct UsedReg {
	size_t pos;
	size_t maxFreeN;
	size_t maxSaveN;
	UsedReg()
		: pos(0)
		, maxFreeN(0)
		, maxSaveN(0)
	{
	}
	int allocRegIdx()
	{
		const int freeTbl[] = {
			3, 4, 5, 6, 7, 0, 1, 2, 24, 25, 26, 27, 28, 29, 30, 31
		};
		maxFreeN = sizeof(freeTbl)/sizeof(freeTbl[0]);
		const int saveTbl[] = {
			8, 9, 10, 11, 12, 13
		};
		maxSaveN = sizeof(saveTbl)/sizeof(saveTbl[0]);
		if (pos < maxFreeN) {
			return freeTbl[pos++];
		}
		if (pos < maxFreeN + maxSaveN) {
			return saveTbl[pos++ - maxFreeN];
		}
		throw std::runtime_error("allocRegIdx");
	}
	size_t getMaxFreeN() { return maxFreeN; }
	size_t getPos() const { return pos; }
};

struct Code : public Xbyak_aarch64::CodeGenerator {
	typedef Xbyak_aarch64::ZReg ZReg;
	typedef Xbyak_aarch64::PReg PReg;
	typedef std::vector<ZReg> ZRegVec;
	ConstVar *constVar;
	typedef void (*VecFunc)(float *dst, const float *src, size_t n);
	VecFunc expf_v;
	VecFunc tanhf_v;
	struct ExpParam {
		ZReg log2_e;
		ZReg not_mask17;
		ZReg one;
		ZReg coeff1;
		ZReg coeff2;
		ZReg expMax;
		ZReg expMin;
		ZReg tanhRange;
		ZReg f1p3; // 1/3
		ExpParam(bool isPreciseTanh, UsedReg& usedReg)
			: log2_e(ZReg(usedReg.allocRegIdx()))
			, not_mask17(ZReg(usedReg.allocRegIdx()))
			, one(ZReg(usedReg.allocRegIdx()))
			, coeff1(ZReg(usedReg.allocRegIdx()))
			, coeff2(ZReg(usedReg.allocRegIdx()))
			, expMax(ZReg(usedReg.allocRegIdx()))
			, expMin(ZReg(usedReg.allocRegIdx()))
			, tanhRange(isPreciseTanh ? usedReg.allocRegIdx() : 0)
			, f1p3(isPreciseTanh ? usedReg.allocRegIdx() : 0)
		{
		}
	};
	Code()
		: Xbyak_aarch64::CodeGenerator(4096 * 2)
		, expf_v(0)
		, tanhf_v(0)
	{
		size_t dataSize = sizeof(ConstVar);
		dataSize = (dataSize + 4095) & ~size_t(4095);
		Xbyak_aarch64::Label constVarL = L();
		constVar = (ConstVar*)getCode();
		constVar->init();
		setSize(dataSize / 4);
		expf_v = getCurr<VecFunc>();
		genExp(constVarL);
		align(16);
		tanhf_v = getCurr<VecFunc>();
		genTanh(constVarL);
		ready();
	}
	// C = regN
	// t[0+i*C] = exp(t[0+i*C]), using t[0+i*C], t[1+i*C], t[2+i*C]
	void genExp1(int regN, int unrollN, const PReg& p, const std::vector<ZReg>& t, const ExpParam& para)
	{
		using namespace Xbyak_aarch64;
		const int C = regN;
		const int N = regN * unrollN;
		for (int i = 0; i < N; i+=C) fmin(t[i+0].s, p, para.expMax.s);
		for (int i = 0; i < N; i+=C) fmax(t[i+0].s, p, para.expMin.s);
		for (int i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+0].s, para.log2_e.s);
		for (int i = 0; i < N; i+=C) {
			movprfx(t[i+1].s, p, t[i+0].s); // clear implicit dependency
			frintm(t[i+1].s, p, t[i+0].s); // floor : float -> float
		}
		for (int i = 0; i < N; i+=C) fcvtzs(t[i+2].s, p, t[i+1].s); // n = float -> int
		for (int i = 0; i < N; i+=C) fsub(t[i+1].s, t[i+0].s, t[i+1].s); // a
		for (int i = 0; i < N; i+=C) fadd(t[i+0].s, t[i+1].s, para.one.s); // b = 1 + a
		for (int i = 0; i < N; i+=C) lsr(t[i+1].s, t[i+0].s, 17); // bL
		for (int i = 0; i < N; i+=C) fexpa(t[i+1].s, t[i+1].s); // c = fexpa(bL)
		for (int i = 0; i < N; i+=C) fscale(t[i+1].s, p, t[i+2].s); // t[i+1] *= 2^n
		for (int i = 0; i < N; i+=C) and_(t[i+2].d, t[i+0].d, para.not_mask17.d);
		for (int i = 0; i < N; i+=C) fsub(t[i+2].s, t[i+0].s, t[i+2].s); // z
		for (int i = 0; i < N; i+=C) {
			movprfx(t[i+0].s, p, para.coeff2.s);
			fmad(t[i+0].s, p, t[i+2].s, para.coeff1.s);
		}
		for (int i = 0; i < N; i+=C) fmad(t[i+0].s, p, t[i+2].s, para.one.s);
		for (int i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+1].s, t[i+0].s);
	}
	// tanh(x) = 1 - 2/(1 + exp(2 x))
	// tanh(x) = (e - 1)/(e + 1) where e = exp(2 x)
	void genTanh1(int regN, int unrollN, const PReg& p, const std::vector<ZReg>& t, const ExpParam& para)
	{
		const int C = regN;
		const int N = regN * unrollN;
		// 2x
		for (int i = 0; i < N; i+=C) fadd(t[i+0].s, t[i+0].s, t[i+0].s);
		// exp(2x)
		genExp1(regN, unrollN, p, t, para);
		// 1+exp(2x)
		for (int i = 0; i < N; i+=C) fadd(t[i+0].s, t[i+0].s, para.one.s);

		// 1/(1+exp(2x))
		// 1st aprox ; a = 1/x + e
		for (int i = 0; i < N; i+=C) frecpe(t[i+1].s, t[i+0].s);
		// 2nd aprox ; a' = (2 - ax)a = 1/x - e^2 x
		for (int i = 0; i < N; i+=C) frecps(t[i+2].s, t[i+0].s, t[i+1].s);
		for (int i = 0; i < N; i+=C) fmul(t[i+2].s, t[i+2].s, t[i+1].s);
		// 3rd aprox ; a'' = (2 - a'x)a'
		for (int i = 0; i < N; i+=C) frecps(t[i+0].s, t[i+0].s, t[i+2].s);
		for (int i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+0].s, t[i+2].s);

		// 2/(1+exp(2x))
		for (int i = 0; i < N; i+=C) fadd(t[i+0].s, t[i+0].s, t[i+0].s);
		// 1-2/(1+exp(2x))
		for (int i = 0; i < N; i+=C) fsub(t[i+0].s, para.one.s, t[i+0].s);
	}
	// f(float *dst, const float *src, size_t n);
	void genFunc(void (Code::*gen1)(int, int, const PReg&, const std::vector<ZReg>&, const ExpParam&), const Xbyak_aarch64::Label& constVarL)
	{
		const int unrollN = 3;
		const int isPreciseTanh = true;
		const int regN = isPreciseTanh ? 4 : 3;

		using namespace Xbyak_aarch64;
		const XReg& dst = x0;
		const XReg& src = x1;
		const XReg& n = x2;

		UsedReg usedReg;

		ExpParam param(isPreciseTanh, usedReg);
		ptrue(p0.s);

		adr(x3, constVarL);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log2_e)));
		cpy(param.log2_e.s, p0/T_z, w4);
		mov(w4, ConstVar::not_mask17);
		cpy(param.not_mask17.s, p0/T_z, w4);
		fcpy(param.one.s, p0/T_z, 1);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, coeff1)));
		cpy(param.coeff1.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, coeff2)));
		cpy(param.coeff2.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, expMax)));
		cpy(param.expMax.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, expMin)));
		cpy(param.expMin.s, p0/T_z, w4);
		if (isPreciseTanh) {
			ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, tanhRange)));
			cpy(param.tanhRange.s, p0/T_z, w4);
			ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, f1p3)));
			cpy(param.f1p3.s, p0/T_z, w4);
		}

		std::vector<ZReg> args;
		for (size_t i = 0; i < regN * unrollN; i++) {
			args.push_back(ZReg(usedReg.allocRegIdx()));
		}
		const size_t maxN = usedReg.getMaxFreeN();
		const size_t pos = usedReg.getPos();
		if (pos > maxN) {
			size_t n = pos - maxN;
			sub(sp, sp, int((n + 1) & ~1) * 64);
			for (int i = 0; i < n; i++) {
				st1w(ZReg(pos + i).s, p0, ptr(sp, i));
			}
		}
		Label skip;
		b(skip);
	Label lp = L();
		ld1w(args[0].s, p0/T_z, ptr(src));
		if (unrollN > 1) ld1w(args[regN].s, p0/T_z, ptr(src, 1));
		if (unrollN > 2) ld1w(args[regN * 2].s, p0/T_z, ptr(src, 2));
		add(src, src, 64 * unrollN);
		(this->*gen1)(regN, unrollN, p0, args, param);
		st1w(args[0].s, p0, ptr(dst));
		if (unrollN > 1) st1w(args[regN].s, p0, ptr(dst, 1));
		if (unrollN > 2) st1w(args[regN * 2].s, p0, ptr(dst, 2));
		add(dst, dst, 64 * unrollN);
		sub(n, n, 16 * unrollN);
	L(skip);
		cmp(n, 16 * unrollN);
		bge(lp);

		Label cond;
		mov(x3, 0);
		b(cond);
	Label lp2 = L();
		ld1w(args[0].s, p1/T_z, ptr(src, x3, LSL, 2));
		(this->*gen1)(regN, 1, p1, args, param);
		st1w(args[0].s, p1, ptr(dst, x3, LSL, 2));
		incd(x3);
	L(cond);
		whilelt(p1.s, x3, n);
		b_first(lp2);
		if (pos > maxN) {
			int n = pos - maxN;
			for (int i = 0; i < n; i++) {
				ld1w(ZReg(pos + i).s, p0, ptr(sp, i));
			}
			add(sp, sp, int((n + 1) & ~1) * 64);
		}
		ret();
	}
	void genExp(const Xbyak_aarch64::Label& constVarL)
	{
		genFunc( &Code::genExp1, constVarL);
	}
	void genTanh(const Xbyak_aarch64::Label& constVarL)
	{
		genFunc(&Code::genTanh1, constVarL);
	}
};

template<size_t dummy = 0>
struct Inst {
	static const Code code;
};

template<size_t dummy>
alignas(32) const Code Inst<dummy>::code;

} // fmath::local

inline void expf_v(float *dst, const float *src, size_t n)
{
	local::Inst<>::code.expf_v(dst, src, n);
}

inline void tanhf_v(float *dst, const float *src, size_t n)
{
	local::Inst<>::code.tanhf_v(dst, src, n);
}

} // fmath2
