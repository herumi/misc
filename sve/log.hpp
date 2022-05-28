#pragma once
/**
	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#if defined(__x86_64__) || defined(_MSC_VER)
	#define FMATH_X64_EMU
#endif

#ifndef FMATH_X64_EMU
#include <xbyak_aarch64/xbyak_aarch64.h>
#include <cmath>
#include <vector>
#endif

//#define FMATH_USE_LOG_TBL

namespace fmath {

namespace local {

#ifndef FMATH_LOCAL_STRUCT
#define FMATH_LOCAL_STRUCT

union fi {
	float f;
	uint32_t i;
};

float u2f(uint32_t x)
{
	fmath::local::fi fi;
	fi.i = x;
	return fi.f;
}

uint32_t f2u(float x)
{
	fmath::local::fi fi;
	fi.f = x;
	return fi.i;
}

#endif

} // fmath::local

namespace local_log {

const bool supportNan = true;
const bool supportLog1p = true;

struct ConstVar {
	static const size_t logN = 9;
	static const size_t L = 5;
	static const size_t LN = 1 << L;
	uint32_t i127shl23;
	uint32_t x7fffff;
	float log2;
	float log1p5;
	float f2div3;
	float logCoeff[logN];
	float sqrt2;
	float inv_sqrt2;
	float f1p32;
	float tbl1[LN];
	float tbl2[LN];
	//
	void init()
	{
		i127shl23 = 127 << 23;
		x7fffff = 0x7fffff;
		log2 = std::log(2.0f);
		log1p5 = std::log(1.5f);
		f2div3 = 2.0f/3;
		sqrt2 = sqrt(2);
		inv_sqrt2 = 1 / sqrt(2);
		f1p32 = 1.0 / 32;
		const float logTbl[logN] = {
			 1.0, // must be 1
#if 1 //#ifdef FMATH_USE_LOG_TBL
			-0.50017447451684279136836,
			 0.3334853298089794502,
#else
			-0.49999985195974875681242,
			 0.33333220526061677705782,
			-0.25004206220486390058000,
			 0.20010985747510067100077,
			-0.16481566812093889672203,
			 0.13988269735629330763020,
			-0.15049504706005165294002,
			 0.14095711402233803479921,
#endif
		};
		for (size_t i = 0; i < logN; i++) {
			logCoeff[i] = logTbl[i];
		}
		for (size_t i = 0; i < LN; i++) {
			local::fi fi;
			fi.i = i127shl23 | ((i*2+1) << (23 - L - 1)); // better approx
//			fi.i = i127shl23 | (i << (23 - L));
			tbl1[i] = 1 / fi.f;
			tbl2[i] = log(tbl1[i]);
		}
	}
};

#ifndef FMATH_X64_EMU
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

#endif

#ifdef FMATH_X64_EMU

struct Code {
	static ConstVar s_constVar;
	ConstVar *constVar;
	Code()
		: constVar(&s_constVar)
	{
		constVar->init();
	}
};
ConstVar Code::s_constVar;

#else

struct Code : public Xbyak_aarch64::CodeGenerator {
	typedef Xbyak_aarch64::ZReg ZReg;
	typedef Xbyak_aarch64::PReg PReg;
	typedef std::vector<ZReg> ZRegVec;
	ConstVar *constVar;
	typedef void (*VecFunc)(float *dst, const float *src, size_t n);
	VecFunc logf_v;
	struct LogParam {
		int regN;
		ZReg i127shl23;
		ZReg x7fffff;
		ZReg log2;
#ifdef FMATH_USE_LOG_TBL
		ZReg sqrt2;
		ZReg inv_sqrt2;
		ZReg f1p32;
#else
		ZReg log1p5;
		ZReg f2div3;
		ZReg tmp;
#endif
		ZReg fNan;
		ZReg fMInf;
		ZRegVec coeffTbl;
		LogParam(UsedReg& usedReg)
#ifdef FMATH_USE_LOG_TBL
			: regN(5)
#else
			: regN((supportNan || supportLog1p) ? 4 : 3)
#endif
			, i127shl23(ZReg(usedReg.allocRegIdx()))
			, x7fffff(ZReg(usedReg.allocRegIdx()))
			, log2(ZReg(usedReg.allocRegIdx()))
#ifdef FMATH_USE_LOG_TBL
			, sqrt2(ZReg(usedReg.allocRegIdx()))
			, inv_sqrt2(ZReg(usedReg.allocRegIdx()))
			, f1p32(ZReg(usedReg.allocRegIdx()))
#else
			, log1p5(ZReg(usedReg.allocRegIdx()))
			, f2div3(ZReg(usedReg.allocRegIdx()))
			, tmp(ZReg(usedReg.allocRegIdx()))
#endif
			, fNan(ZReg(supportNan ? usedReg.allocRegIdx() : 0))
			, fMInf(ZReg(supportNan ? usedReg.allocRegIdx() : 0))
		{
#ifdef FMATH_USE_LOG_TBL
			const int n = 3;
#else
			const int n = (int)ConstVar::logN;
#endif
			for (int i = 0; i < n; i++) {
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
	void genLog1(const LogParam& para, int unrollN, const std::vector<ZReg>& t)
	{
		using namespace Xbyak_aarch64;
		const int C = para.regN;
		const int N = C * unrollN;
#ifdef FMATH_USE_LOG_TBL
#if 1
		Label tbl1L, tbl2L, skipL;
		b(skipL);
		align(128);
	L(tbl1L);
		for (size_t i = 0; i < ConstVar::LN; i++) {
			local::fi fi;
			fi.i = (127 << 23) | (i << (23 - ConstVar::L));
			fi.f = std::sqrt(2) / fi.f;
			dd(fi.i);
		}
	L(tbl2L);
		const float *tbl1Addr = (const float *)tbl1L.getAddress();
		for (size_t i = 0; i < ConstVar::LN; i++) {
			local::fi fi;
			fi.f = std::log(tbl1Addr[i]);
			dd(fi.i);
		}
	L(skipL);
#endif
		if (supportNan || supportLog1p) {
			for (int i = 0; i < N; i+=C) mov(t[i+4].s, p0, t[i+0].s);
		}

		for (int i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+0].s, para.sqrt2.s);
		for (int i = 0; i < N; i+=C) sub(t[i+1].s, t[i+0].s, para.i127shl23.s);
		for (int i = 0; i < N; i+=C) asr(t[i+1].s, t[i+1].s, 23); // n
		for (int i = 0; i < N; i+=C) scvtf(t[i+1].s, p0, t[i+1].s); // int -> float
		for (int i = 0; i < N; i+=C) and_(t[i+0].s, p0, para.x7fffff.s);
		for (int i = 0; i < N; i+=C) asr(t[i+2].s, t[i+0].s, 23 - ConstVar::L); // d
		for (int i = 0; i < N; i+=C) lsl(t[i+2].s, t[i+2].s, 2); // d *= 4
		for (int i = 0; i < N; i+=C) orr(t[i+0].s, p0, para.i127shl23.s); // y
		for (int i = 0; i < N; i+=C) fmul(t[i+0].s, t[i+0].s, para.inv_sqrt2.s);
//		add(x4, x3, (uint32_t)offsetof(ConstVar, tbl1));
		adr(x4, tbl1L);
		for (int i = 0; i < N; i+=C) ld1w(t[i+3].s, p0, ptr(x4, t[i+2].s, SXTW)); // f
		for (int i = 0; i < N; i+=C) fnmsb(t[i+0].s, p0, t[i+3].s, para.coeffTbl[0].s); // y = y * f - 1
//		add(x4, x3, (uint32_t)offsetof(ConstVar, tbl2));
		adr(x4, tbl2L);
		for (int i = 0; i < N; i+=C) ld1w(t[i+2].s, p0, ptr(x4, t[i+2].s, SXTW)); // h
		if (supportLog1p) {
			for (int i = 0; i < N; i+=C) {
				fsub(t[i+3].s, t[i+4].s, para.coeffTbl[0].s); // x-1
				facge(p1.s, p0, para.f1p32.s, t[i+3].s); // 1/32 >= abs(x-1)
				mov(t[i+0].s, p1, t[i+3].s);
				eor(t[i+2].s, p1, t[i+2].s);
			}
		}
		for (int i = 0; i < N; i+=C) fnmsb(t[i+1].s, p0, para.log2.s, t[i+2].s); // x = n * log2 - h
		for (int i = 0; i < N; i+=C) {
			movprfx(t[i+2].s, p0, para.coeffTbl[2].s);
			fmad(t[i+2].s, p0, t[i+0].s, para.coeffTbl[1].s); // f
		}
		for (int i = 0; i < N; i+=C) fmad(t[i+2].s, p0, t[i+0].s, para.coeffTbl[0].s); // f * y + 1
		for (int i = 0; i < N; i+=C) fmad(t[i+0].s, p0, t[i+2].s, t[i+1].s); // y * f + x
		if (supportNan) {
			for (int i = 0; i < N; i+=C) {
				fcmlt(p1.s, p0, t[i+4].s, 0); // neg
				mov(t[i+0].s, p1, para.fNan.s);
				fcmeq(p1.s, p0, t[i+4].s, 0); // = 0
				mov(t[i+0].s, p1, para.fMInf.s);
			}
		}
#else
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
		if (supportLog1p) {
			for (int i = 0; i < N; i+=C) {
				fsub(t[i+2].s, t[i+3].s, para.coeffTbl[0].s); // x-1
				fcpy(para.tmp.s, p0, 1.0/8);
				facge(p1.s, p0, para.tmp.s, t[i+2].s); // 1/8 >= abs(x-1)
				mov(t[i+0].s, p1, t[i+2].s);
				eor(t[i+1].s, p1, t[i+1].s);
			}
		}
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
#endif
	}
	// f(float *dst, const float *src, size_t n);
	void genFunc(void (Code::*gen1)(const LogParam&, int unrollN, const std::vector<ZReg>&), const Xbyak_aarch64::Label& constVarL)
	{
		using namespace Xbyak_aarch64;
		const XReg& dst = x0;
		const XReg& src = x1;
		const XReg& n = x2;

		UsedReg usedReg;

		LogParam param(usedReg);
		const int unrollN = 3;
		const int regN = param.regN;

		ptrue(p0.s);

		std::vector<ZReg> args;
		for (int i = 0; i < regN * unrollN; i++) {
			args.push_back(ZReg(usedReg.allocRegIdx()));
		}
		const size_t pos = usedReg.getPos();
		if (pos > maxFreeN) {
			int n = pos - maxFreeN;
			for (int i = 0; i < n; i++) {
				int idx = saveTbl[i];
				sub(sp, sp, 64);
				st1w(ZReg(idx).s, p0, ptr(sp));
			}
		}
		adr(x3, constVarL);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, i127shl23)));
		cpy(param.i127shl23.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, x7fffff)));
		cpy(param.x7fffff.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log2)));
		cpy(param.log2.s, p0/T_z, w4);
#ifdef FMATH_USE_LOG_TBL
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, sqrt2)));
		cpy(param.sqrt2.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, inv_sqrt2)));
		cpy(param.inv_sqrt2.s, p0/T_z, w4);
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, f1p32)));
		cpy(param.f1p32.s, p0/T_z, w4);
#else
		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, log1p5)));
		cpy(param.log1p5.s, p0/T_z, w4);

		ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, f2div3)));
		cpy(param.f2div3.s, p0/T_z, w4);
#endif

		if (supportNan) {
			mov(w4, 0x7fc00000); // Nan
			cpy(param.fNan.s, p0/T_z, w4);
			mov(w4, 0xff800000); // -Inf
			cpy(param.fMInf.s, p0/T_z, w4);
		}

		for (int i = 0; i < (int)param.coeffTbl.size(); i++) {
			ldr(w4, ptr(x3, (uint32_t)offsetof(ConstVar, logCoeff) + i * 4));
			cpy(param.coeffTbl[i].s, p0/T_z, w4);
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
		mov(x5, 0);
		b(cond);
	Label lp2 = L();
		ld1w(args[0].s, p0/T_z, ptr(src, x5, LSL, 2));
		(this->*gen1)(param, 1, args);
		st1w(args[0].s, p0, ptr(dst, x5, LSL, 2));
		incd(x5);
	L(cond);
		whilelt(p0.s, x5, n);
		b_first(lp2);

		if (pos > maxFreeN) {
			int n = pos - maxFreeN;
			ptrue(p0.s);
			for (int i = 0; i < n; i++) {
				int idx = saveTbl[n - 1 - i];
				ld1w(ZReg(idx).s, p0, ptr(sp));
				add(sp, sp, 64);
			}
		}
		ret();
	}
	void genLog(const Xbyak_aarch64::Label& constVarL)
	{
		genFunc(&Code::genLog1, constVarL);
	}
};
#endif

template<size_t dummy = 0>
struct Inst {
	static const Code code;
};

template<size_t dummy>
alignas(32) const Code Inst<dummy>::code;

} // fmath::local_log

#ifndef FMATH_X64_EMU
inline void logf_v(float *dst, const float *src, size_t n)
{
	local_log::Inst<>::code.logf_v(dst, src, n);
}

inline float logf(float x)
{
	float ret;
	logf_v(&ret, &x, 1);
	return ret;
}
#endif

} // fmath2
