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

struct ConstVar {
	static const size_t expN = 5;
	float expMin; // exp(expMin) = 0
	float expMax; // exp(expMax) = inf
	float log2; // log(2)
	float log2_e; // log_2(e) = 1 / log2
	float expCoeff[expN]; // near to 1/(i + 1)!
	uint32_t geluC1;
	uint32_t geluC2;
	void init()
	{
		expMin = cvt(0xc2aeac50);
		expMax = cvt(0x42b17218);
		log2 = std::log(2.0f);
		log2_e = 1.0f / log2;
		// maxe=1.938668e-06
		const uint32_t expTbl[expN] = {
			0x3f800000,
			0x3effff12,
			0x3e2aaa56,
			0x3d2b89cc,
			0x3c091331,
		};
		for (size_t i = 0; i < expN; i++) {
			expCoeff[i] = cvt(expTbl[i]);
		}
		geluC1 = 0x3fcc422a;
		geluC2 = 0x3d922279;
	}
};

struct Code : public Xbyak::CodeGenerator {
	typedef Xbyak::ZReg ZReg;
	typedef Xbyak::PReg PReg;
	ConstVar *constVar;
	typedef void (*VecFunc)(float *dst, const float *src, size_t n);
	VecFunc gelu_v;
	Code()
		: Xbyak::CodeGenerator(4096 * 2)
		, gelu_v(0)
	{
		size_t dataSize = sizeof(ConstVar);
		dataSize = (dataSize + 4095) & ~size_t(4095);
		Xbyak::Label constVarL = L();
		constVar = (ConstVar*)getCode();
		constVar->init();
		setSize(dataSize / 4);
		gelu_v = getCurr<VecFunc>();
		genGelu(constVarL);
		ready();
	}
	// tz0 = exp(tz0)
	// use tz0, tz1, tz2, tz3
	void gen1Gelu(const PReg& p, const ZReg& tz0, const ZReg& tz1, const ZReg& tz2, const ZReg& tz3, const ZReg& expMin, const ZReg& expMax, const ZReg& log2, const ZReg& log2_e, const ZReg expCoeff[5], const ZReg& C1, const ZReg& C2)
	{
		// fmad(dn, m, a) ; dn = a + dn * m
		// x^2
		fmul(tz3.s, tz0.s, tz0.s);
		fmad(tz3.s, p, C2.s, C1.s);
		// Cx
		fmul(tz3.s, tz3.s, tz0.s);

		// exp(Cx)
		fmin(tz3.s, p, expMax.s);
		fmax(tz3.s, p, expMin.s);
		fmul(tz3.s, tz3.s, log2_e.s);
		frintn(tz2.s, p, tz3.s); // rounding : float -> float
		fcvtzs(tz1.s, p, tz2.s); // float -> int
		fsub(tz2.s, tz3.s, tz2.s);
		fmul(tz2.s, tz2.s, log2.s);
		movprfx(tz3.s, p, expCoeff[4].s);
		fmad(tz3.s, p, tz2.s, expCoeff[3].s);
		fmad(tz3.s, p, tz2.s, expCoeff[2].s);
		fmad(tz3.s, p, tz2.s, expCoeff[1].s);
		fmad(tz3.s, p, tz2.s, expCoeff[0].s);
		fmad(tz3.s, p, tz2.s, expCoeff[0].s);
		fscale(tz3.s, p, tz1.s); // tz3 *= 2^tz1

		// 1 + exp(Cx)
		fadd(tz3.s, tz3.s, expCoeff[0].s);
#if 1
		// x / (1 + exp(Cx))
		movprfx(tz1.s, p, tz0.s);
		fdiv(tz1.s, p, tz3.s);
		// G(x) = x(1 - 1/(1 + exp(Cx)))
		fsub(tz0.s, tz0.s, tz1.s);
#else
		// 1 / (1 + exp(Cx))
		frecpe(tz1.s, tz3.s);
		frecps(tz3.s, tz3.s, tz1.s);
		fmul(tz3.s, tz3.s, tz1.s);
		// 1 - 1/(1 + exp(Cx))
		fsub(tz3.s, expCoeff[0].s, tz3.s);
		// G(x) = x(1 - 1/(1 + exp(Cx)))
		fmul(tz0.s, tz0.s, tz3.s);
#endif
	}
	// gelu_v(float *dst, const float *src, size_t n);
	void genGelu(const Xbyak::Label& constVarL)
	{
		using namespace Xbyak;
		const XReg& dst = x0;
		const XReg& src = x1;
		const XReg& n = x2;

		// setup constant
		const ZReg& expMin = z4;
		const ZReg& expMax = z5;
		const ZReg& log2 = z6;
		const ZReg& log2_e = z7;
		const ZReg& geluC1 = z8;
		const ZReg& geluC2 = z9;
		const ZReg expCoeff[] = {
			z10, z11, z12, z13, z14,
		};
		const size_t saveN = 14 - 8 + 1; /* z8..z14 */
		const size_t adj = saveN / 2;
		sub(sp, sp, saveN * 64);
		add(x3, sp, adj * 64);
		ptrue(p0.s);
		for (size_t i = 0; i < saveN; i++) {
			st1w(ZReg(i + 8).s, p0, ptr(x3, int(i - adj)));
		}

		adr(x3, constVarL);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, expMin)));
		cpy(expMin.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, expMax)));
		cpy(expMax.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log2)));
		cpy(log2.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log2_e)));
		cpy(log2_e.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, geluC1)));
		cpy(geluC1.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, geluC2)));
		cpy(geluC2.s, p0/T_z, w4);
		for (size_t i = 0; i < ConstVar::expN; i++) {
			ldr(w4, ptr(x3, uint32_t(offsetof(ConstVar, expCoeff[0]) + sizeof(float) * i)));
			cpy(expCoeff[i].s, p0/T_z, w4);
		}
		Label skip;
		b(skip);
	Label lp = L();
		ld1w(z0.s, p0/T_z, ptr(src));
		add(src, src, 64);
		gen1Gelu(p0, z0, z1, z2, z3, expMin, expMax, log2, log2_e, expCoeff, geluC1, geluC2);
		st1w(z0.s, p0, ptr(dst));
		add(dst, dst, 64);
		sub(n, n, 16);
	L(skip);
		cmp(n, 16);
		bge(lp);

		mov(x3, 0);
		whilelt(p1.s, x3, n);
		ld1w(z0.s, p1/T_z, ptr(src));
		gen1Gelu(p1, z0, z1, z2, z3, expMin, expMax, log2, log2_e, expCoeff, geluC1, geluC2);
		st1w(z0.s, p1, ptr(dst));

		add(x3, sp, adj * 64);
		for (size_t i = 0; i < saveN; i++) {
			ld1w(ZReg(i + 8).s, p0, ptr(x3, int(i - adj)));
		}
		add(sp, sp, saveN * 64);
		ret();
	}
};

template<size_t dummy = 0>
struct Inst {
	static const Code code;
};

template<size_t dummy>
alignas(32) const Code Inst<dummy>::code;

} // fmath::local

inline void gelu_v(float *dst, const float *src, size_t n)
{
	local::Inst<>::code.gelu_v(dst, src, n);
}

} // fmath2
