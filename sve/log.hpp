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

const bool supportNan = true;
const bool supportLog1p = true;

struct ConstVar {
	static const size_t logN = 9;
	uint32_t i127shl23;
	uint32_t x7fffff;
	float log2;
	float log1p5;
	float f2div3;
	float fNan;
	float fMInf;
	float logCoeff[logN];
	//
	void init()
	{
		i127shl23 = 127 << 23;
		x7fffff = 0x7fffff;
		log2 = std::log(2.0f);
		log1p5 = std::log(1.5f);
		f2div3 = 2.0f/3;
		fi fi;
		fi.i = 0x7fc00000;
		fNan = fi.f;
		fi.i = 0xff800000;
		fMInf = fi.f;
		const float logTbl[logN] = {
			 1.0, // must be 1
			-0.49999985195974875681242,
			 0.33333220526061677705782,
			-0.25004206220486390058000,
			 0.20010985747510067100077,
			-0.16481566812093889672203,
			 0.13988269735629330763020,
			-0.15049504706005165294002,
			 0.14095711402233803479921,
		};
		for (size_t i = 0; i < logN; i++) {
			logCoeff[i] = logTbl[i];
		}
	}
};

const int freeTbl[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 24, 25, 26, 27, 28, 29, 30, 31
};
static const size_t maxFreeN = sizeof(freeTbl)/sizeof(freeTbl[0]);

const int saveTbl[] = {
	8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
};
static const size_t maxSaveN = sizeof(saveTbl)/sizeof(saveTbl[0]);

struct UsedReg {
	size_t pos;
	UsedReg()
		: pos(0)
	{
	}
	int allocRegIdx()
	{
		if (pos < maxFreeN) {
			return freeTbl[pos++];
		}
		if (pos < maxFreeN + maxSaveN) {
			return saveTbl[pos++ - maxFreeN];
		}
		throw std::runtime_error("allocRegIdx");
	}
	size_t getPos() const { return pos; }
};

struct Code : public Xbyak_aarch64::CodeGenerator {
	typedef Xbyak_aarch64::ZReg ZReg;
	typedef Xbyak_aarch64::PReg PReg;
	typedef std::vector<ZReg> ZRegVec;
	ConstVar *constVar;
	typedef void (*VecFunc)(float *dst, const float *src, size_t n);
	VecFunc logf_v;
	struct ExpParam {
		int regN;
		ZReg i127shl23;
		ZReg x7fffff;
		ZReg log2;
		ZReg log1p5;
		ZReg f2div3;
		ZReg fNan;
		ZReg fMInf;
		ZRegVec coeffTbl;
		ExpParam(UsedReg& usedReg)
			: regN((supportNan || supportLog1p) ? 4 : 3)
			, i127shl23(ZReg(usedReg.allocRegIdx()))
			, x7fffff(ZReg(usedReg.allocRegIdx()))
			, log2(ZReg(usedReg.allocRegIdx()))
			, log1p5(ZReg(usedReg.allocRegIdx()))
			, f2div3(ZReg(usedReg.allocRegIdx()))
			, fNan(ZReg(supportNan ? usedReg.allocRegIdx() : 0))
			, fMInf(ZReg(supportNan ? usedReg.allocRegIdx() : 0))
		{
			for (int i = 0; i < (int)ConstVar::logN; i++) {
				coeffTbl.push_back(ZReg(usedReg.allocRegIdx()));
			}
		}
	};
	Code()
		: Xbyak_aarch64::CodeGenerator(4096 * 2)
		, logf_v(0)
	{
		size_t dataSize = sizeof(ConstVar);
		dataSize = (dataSize + 4095) & ~size_t(4095);
		Xbyak_aarch64::Label constVarL = L();
		constVar = (ConstVar*)getCode();
		constVar->init();
		setSize(dataSize / 4);
		logf_v = getCurr<VecFunc>();
		genLog(constVarL);
		align(16);
		ready();
	}
	// assume p0 is all true
	// use p1, ..., p_unrollN if supportLog1p
	void genLog1(const ExpParam& para, int unrollN, const std::vector<ZReg>& t)
	{
		using namespace Xbyak_aarch64;
		const int C = para.regN;
		const int N = C * unrollN;
		if (supportNan || supportLog1p) {
			for (int i = 0; i < N; i+=C) mov(t[i+3].s, p0, t[i+0].s);
		}
		for (int i = 0; i < N; i+=C) sub(t[i+1].s, t[i+0].s, para.i127shl23.s);
		for (int i = 0; i < N; i+=C) asr(t[i+1].s, t[i+1].s, 23);
		// int -> float
		for (int i = 0; i < N; i+=C) scvtf(t[i+1].s, p0, t[i+1].s);
		for (int i = 0; i < N; i+=C) and_(t[i+0].s, p0, para.x7fffff.s);
		for (int i = 0; i < N; i+=C) orr(t[i+0].s, p0, para.i127shl23.s);

		// fnmsb(a, b, c) = a * b - c
		for (int i = 0; i < N; i+=C) fnmsb(t[i+0].s, p0, para.f2div3.s, para.coeffTbl[0].s);
		for (int i = 0; i < N; i+=C) fmad(t[i+1].s, p0, para.log2.s, para.log1p5.s);
#if 0
		if (supportLog1p) {
			for (int i = 0; i < N; i+=C) {
				fcpy(t[i+2].s, p0, 1.0);
				fsub(t[i+2].s, t[i+3].s, t[i+2].s); // x - 1
				fabs(t[i+2].s, p0, t[i+2].s);
				fclt(p1.s, p0, t[i+2].s, 1.0/8);
				mov(t[i+0].s, p1, t[i+2].s);
				fcpy(t[i+2].s, p0, 1.0);
				mov(t[i+1].s, p1, t[i+2].s);
			}
		}
#endif
		const int logN = (int)ConstVar::logN;
		// fmad(a, b, c) ; a = a * b + c
		for (int i = 0; i < N; i+=C) {
			movprfx(t[i+2].s, p0, para.coeffTbl[logN - 1].s);
			fmad(t[i+2].s, p0, t[i+0].s, para.coeffTbl[logN - 2].s);
		}
		for (int j = logN - 3; j >= 0; j--) {
			for (int i = 0; i < N; i+=C) fmad(t[i+2].s, p0, t[i+0].s, para.coeffTbl[j].s);
		}
		// a * x + e
		for (int i = 0; i < N; i+=C) fmad(t[i+0].s, p0, t[i+2].s, t[i+1].s);
		if (supportNan) {
			for (int i = 0; i < N; i+=C) {
				fcmlt(p1.s, p0, t[i+3].s, 0); // neg
				mov(t[i+0].s, p1, para.fNan.s);
				fcmeq(p1.s, p0, t[i+3].s, 0); // = 0
				mov(t[i+0].s, p1, para.fMInf.s);
			}
		}
	}
	// f(float *dst, const float *src, size_t n);
	void genFunc(void (Code::*gen1)(const ExpParam&, int unrollN, const std::vector<ZReg>&), const Xbyak_aarch64::Label& constVarL)
	{
		using namespace Xbyak_aarch64;
		const XReg& dst = x0;
		const XReg& src = x1;
		const XReg& n = x2;

		UsedReg usedReg;

		ExpParam param(usedReg);
		const int unrollN = 1;
		const int regN = param.regN;
		ptrue(p0.s);

		adr(x3, constVarL);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, i127shl23)));
		cpy(param.i127shl23.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, x7fffff)));
		cpy(param.x7fffff.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log2)));
		cpy(param.log2.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log1p5)));
		cpy(param.log1p5.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, f2div3)));
		cpy(param.f2div3.s, p0/T_z, w4);

		if (supportNan) {
			ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, fNan)));
			cpy(param.fNan.s, p0/T_z, w4);
			ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, fMInf)));
			cpy(param.fMInf.s, p0/T_z, w4);
		}

		for (int i = 0; i < (int)ConstVar::logN; i++) {
			ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, logCoeff) + i * 4));
			cpy(param.coeffTbl[i].s, p0/T_z, w4);
		}

		std::vector<ZReg> args;
		for (int i = 0; i < regN * unrollN; i++) {
			args.push_back(ZReg(usedReg.allocRegIdx()));
		}
		const size_t pos = usedReg.getPos();
		if (pos > maxFreeN) {
			int n = pos - maxFreeN;
			sub(sp, sp, int((n + 1) & ~1) * 64);
			for (int i = 0; i < n; i++) {
				int idx = saveTbl[i];
				st1w(ZReg(idx).s, p0, ptr(sp, i));
			}
		}
		Label skip;
		b(skip);
	Label lp = L();
		ld1w(args[0].s, p0/T_z, ptr(src));
		if (unrollN > 1) ld1w(args[regN].s, p0/T_z, ptr(src, 1));
		if (unrollN > 2) ld1w(args[regN * 2].s, p0/T_z, ptr(src, 2));
		add(src, src, 64 * unrollN);
		(this->*gen1)(param, unrollN, args);
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
		ld1w(args[0].s, p0/T_z, ptr(src, x3, LSL, 2));
		(this->*gen1)(param, 1, args);
		st1w(args[0].s, p0, ptr(dst, x3, LSL, 2));
		incd(x3);
	L(cond);
		whilelt(p0.s, x3, n);
		b_first(lp2);
		if (pos > maxFreeN) {
			int n = pos - maxFreeN;
			ptrue(p0.s);
			for (int i = 0; i < n; i++) {
				int idx = saveTbl[i];
				ld1w(ZReg(idx).s, p0, ptr(sp, i));
			}
			add(sp, sp, int((n + 1) & ~1) * 64);
		}
		ret();
	}
	void genLog(const Xbyak_aarch64::Label& constVarL)
	{
		genFunc(&Code::genLog1, constVarL);
	}
};

template<size_t dummy = 0>
struct Inst {
	static const Code code;
};

template<size_t dummy>
alignas(32) const Code Inst<dummy>::code;

} // fmath::local

inline void logf_v(float *dst, const float *src, size_t n)
{
	local::Inst<>::code.logf_v(dst, src, n);
}

inline float logf(float x)
{
	float xs[16];
	xs[0] = x;
	logf_v(xs, xs, 1);
	return xs[0];
}

} // fmath2
