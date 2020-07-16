#pragma once
/**
	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <xbyak_aarch64/xbyak_aarch64.h>
#include <cmath>

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
	static const uint32_t not_mask17= ~((1u << 17) - 1);
	//
	void init()
	{
		log2_e = 1.0f / std::log(2.0f);
		one = 1.0f;
		coeff1 = 0.6931473921;
		coeff2 = 0.2413862043;
	}
};

struct Code : public Xbyak::CodeGenerator {
	typedef Xbyak::ZReg ZReg;
	typedef Xbyak::PReg PReg;
	ConstVar *constVar;
	typedef void (*VecFunc)(float *dst, const float *src, size_t n);
	VecFunc expf_v;
	VecFunc tanhf_v;
	static const int regN = 4;
	static const int maxUnrollN = 3;
	static const size_t allN = maxUnrollN * regN;
	struct ExpParam {
		ZReg log2_e;
		ZReg not_mask17;
		ZReg one;
		ZReg coeff1;
		ZReg coeff2;
		ExpParam(int i0, int i1, int i2, int i3, int i4)
			: log2_e(i0)
			, not_mask17(i1)
			, one(i2)
			, coeff1(i3)
			, coeff2(i4)
		{
		}
	};
	Code()
		: Xbyak::CodeGenerator(4096 * 2)
		, expf_v(0)
		, tanhf_v(0)
	{
		size_t dataSize = sizeof(ConstVar);
		dataSize = (dataSize + 4095) & ~size_t(4095);
		Xbyak::Label constVarL = L();
		constVar = (ConstVar*)getCode();
		constVar->init();
		setSize(dataSize / 4);
		expf_v = getCurr<VecFunc>();
		genExpSVE(constVarL);
		align(16);
		tanhf_v = getCurr<VecFunc>();
//		genTanhSVE(constVarL);
		ready();
	}
	// C = regN
	// t[0+i*C] = exp(t[0+i*C]), using t[0+i*C], t[1+i*C], t[2+i*C], t[3+i*C]
	void genExp1SVE(int unrollN, const PReg& p, const std::array<ZReg, allN>& t, const ExpParam& para)
	{
		const int C = regN;
		const int N = regN * unrollN;
		assert(N <= allN);
		for (size_t i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+0].s, para.log2_e.s);
		for (size_t i = 0; i < N; i+=C) frintm(t[i+1].s, p, t[i+0].s); // floor : float -> float
		for (size_t i = 0; i < N; i+=C) fcvtzs(t[i+2].s, p, t[i+1].s); // n = float -> int
		for (size_t i = 0; i < N; i+=C) fsub(t[i+1].s, t[i+0].s, t[i+1].s); // a
		for (size_t i = 0; i < N; i+=C) fadd(t[i+1].s, t[i+1].s, para.one.s); // b = 1 + a
		for (size_t i = 0; i < N; i+=C) lsr(t[i+3].s, t[i+1].s, 17); // bL
		for (size_t i = 0; i < N; i+=C) fexpa(t[i+3].s, t[i+3].s); // c = fexpa(bL)
		for (size_t i = 0; i < N; i+=C) fscale(t[i+3].s, p, t[i+2].s); // t[i+3] *= 2^n
		for (size_t i = 0; i < N; i+=C) and_(t[i+2].d, t[i+1].d, para.not_mask17.d);
		for (size_t i = 0; i < N; i+=C) fsub(t[i+2].s, t[i+1].s, t[i+2].s); // z
		for (size_t i = 0; i < N; i+=C) {
			movprfx(t[i+0].s, p, para.coeff2.s);
			fmad(t[i+0].s, p, t[i+2].s, para.coeff1.s);
		}
		for (size_t i = 0; i < N; i+=C) fmad(t[i+0].s, p, t[i+2].s, para.one.s);
		for (size_t i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+3].s, t[i+0].s);
	}
	// f(float *dst, const float *src, size_t n);
	template<size_t N>
	void genFunc(void (Code::*gen1)(int unrollN, const PReg&, const std::array<ZReg, N>&, const ExpParam&), const Xbyak::Label& constVarL)
	{
		using namespace Xbyak;
		const XReg& dst = x0;
		const XReg& src = x1;
		const XReg& n = x2;

		ExpParam param(3, 4, 5, 6, 7);
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

		const auto args = std::array<ZReg, allN>{z0, z1, z2, z24, z25, z26, z27, z28, z29, z30, z31, z23};
		const int unrollN = 3;
		if (unrollN == 3) {
			sub(sp, sp, 64);
			st1w(z23.s, p0, ptr(sp));
		}
		Label skip;
		b(skip);
	Label lp = L();
		ld1w(z0.s, p0/T_z, ptr(src));
		if (unrollN > 1) ld1w(args[regN].s, p0/T_z, ptr(src, 1));
		if (unrollN > 2) ld1w(args[regN * 2].s, p0/T_z, ptr(src, 2));
		add(src, src, 64 * unrollN);
		(this->*gen1)(unrollN, p0, args, param);
		st1w(z0.s, p0, ptr(dst));
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
		ld1w(z0.s, p1/T_z, ptr(src, x3, LSL, 2));
		(this->*gen1)(1, p1, args, param);
		st1w(z0.s, p1, ptr(dst, x3, LSL, 2));
		incd(x3);
	L(cond);
		whilelt(p1.s, x3, n);
		b_first(lp2);
		if (unrollN == 3) {
			ld1w(z23.s, p0, ptr(sp));
			add(sp, sp, 64);
		}
		ret();
	}
	void genExpSVE(const Xbyak::Label& constVarL)
	{
		genFunc(&Code::genExp1SVE, constVarL);
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

} // fmath2
