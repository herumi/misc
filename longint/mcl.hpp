#pragma once

#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

#include <stdio.h>
#include <assert.h>
#include "mcl.h"

using namespace Xbyak;
using namespace Xbyak::util;

typedef uint64_t Unit;

const char *pStr = "0x9401ff90f28bffb0c610fb10bf9e0fefd59211629a7991563c5e468d43ec9cfe1549fd59c20ab5b9a7cda7f27a0067b8303eeb4b31555cf4f24050ed155555cd7fa7a5f8aaaaaaad47ede1a6aaaaaaaab69e6dcb";

void3u mcl_mulPre;

template<class T>
T getMontgomeryCoeff(T pLow)
{
	T ret = 0;
	T t = 0;
	T x = 1;
	for (size_t i = 0; i < sizeof(T) * 8; i++) {
		if ((t & 1) == 0) {
			t += pLow;
			ret += x;
		}
		t >>= 1;
		x <<= 1;
	}
	return ret;
}


struct Code : Xbyak::CodeGenerator {
	const Reg64& gp0;
	const Reg64& gp1;
	const Reg64& gp2;
	const Reg64& gt0;
	const Reg64& gt1;
	const Reg64& gt2;
	const Reg64& gt3;
	const Reg64& gt4;
	const Reg64& gt5;
	const Reg64& gt6;
	const Reg64& gt7;
	const Reg64& gt8;
	const Reg64& gt9;
	static const int N = 11;
	Unit rp_;
	Unit p_[N];
	int bitSize;

	/*
		@param op [in] ; use op.p, op.N, op.isFullBit
	*/
	Code()
		: CodeGenerator(4096 * 9, Xbyak::DontSetProtectRWE)
#ifdef XBYAK64_WIN
		, gp0(rcx)
		, gp1(r11)
		, gp2(r8)
		, gt0(r9)
		, gt1(r10)
		, gt2(rdi)
		, gt3(rsi)
#else
		, gp0(rdi)
		, gp1(rsi)
		, gp2(r11)
		, gt0(rcx)
		, gt1(r8)
		, gt2(r9)
		, gt3(r10)
#endif
		, gt4(rbx)
		, gt5(rbp)
		, gt6(r12)
		, gt7(r13)
		, gt8(r14)
		, gt9(r15)
		, rp_(0)
		, p_()
	{
	}
	void init()
	{
		mpz_class p(pStr);
		bitSize = mcl::gmp::getBitSize(p);
		mcl::gmp::getArray(p_, N, p);
		rp_ = getMontgomeryCoeff(p_[0]);
		printf("bitSize=%d rp_=%016llx\n", bitSize, (long long)rp_);

		align(16);
		mcl_mulPre = getCurr<void3u>();
		mulPre11();

		setProtectModeRE(); // set read/exec memory
	}
private:
	Code(const Code&);
	void operator=(const Code&);
	// [gp0] <- [gp1] * [gp2]
	void mulPre11()
	{
		StackFrame sf(this, 3, 10 | UseRDX);
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
		const Reg64& t8 = sf.t[8];
		const Reg64& t9 = sf.t[9];
		const Reg64& ta = px;
		const Reg64& tb = rsp;
		movq(xm0, px);
		movq(xm1, rsp);

		mulPack(pz, xm0, 8 * 0, py, Pack(ta, t9, t8, t7, t6, t5, t4, t3, t2, t1, t0));

		mulPackAdd(pz, xm0, 8 * 1, py, tb, Pack(ta, t9, t8, t7, t6, t5, t4, t3, t2, t1, t0));
		mulPackAdd(pz, xm0, 8 * 2, py, t0, Pack(tb, ta, t9, t8, t7, t6, t5, t4, t3, t2, t1));
		mulPackAdd(pz, xm0, 8 * 3, py, t1, Pack(t0, tb, ta, t9, t8, t7, t6, t5, t4, t3, t2));
		mulPackAdd(pz, xm0, 8 * 4, py, t2, Pack(t1, t0, tb, ta, t9, t8, t7, t6, t5, t4, t3));
		mulPackAdd(pz, xm0, 8 * 5, py, t3, Pack(t2, t1, t0, tb, ta, t9, t8, t7, t6, t5, t4));
		mulPackAdd(pz, xm0, 8 * 6, py, t4, Pack(t3, t2, t1, t0, tb, ta, t9, t8, t7, t6, t5));
		mulPackAdd(pz, xm0, 8 * 7, py, t5, Pack(t4, t3, t2, t1, t0, tb, ta, t9, t8, t7, t6));
		mulPackAdd(pz, xm0, 8 * 8, py, t6, Pack(t5, t4, t3, t2, t1, t0, tb, ta, t9, t8, t7));
		mulPackAdd(pz, xm0, 8 * 9, py, t7, Pack(t6, t5, t4, t3, t2, t1, t0, tb, ta, t9, t8));
		mulPackAdd(pz, xm0, 8 *10, py, t8, Pack(t7, t6, t5, t4, t3, t2, t1, t0, tb, ta, t9));
		store_mr(pz + 8 * 11, Pack(t8, t7, t6, t5, t4, t3, t2, t1, t0, tb, ta));
		movq(rsp, xm1);
	}
	/*
		[pd:pz[0]] <- py[n-1..0] * px[0]
	*/
	void mulPack(const Reg64& pz, const Xmm& px, int offset, const RegExp& py, const Pack& pd)
	{
		const Reg64& a = rax;
		const Reg64& d = rdx;
		movq(d, px);
		mov(d, ptr [d + offset]);
		mulx(pd[0], a, ptr [py + 8 * 0]);
		mov(ptr [pz + offset], a);
		xor_(a, a);
		for (size_t i = 1; i < pd.size(); i++) {
			mulx(pd[i], a, ptr [py + 8 * i]);
			adcx(pd[i - 1], a);
		}
		adc(pd[pd.size() - 1], 0);
	}
	/*
		xmm = px
		[hi:Pack(d_(n-1), .., d1):pz[0]] <- Pack(d_(n-1), ..., d0) + py[n-1..0] * px[0]
	*/
	void mulPackAdd(const RegExp& pz, const Xmm& px, int offset, const RegExp& py, const Reg64& hi, const Pack& pd)
	{
		const Reg64& a = rax;
		const Reg64& d = rdx;
		movq(d, px);
		mov(d, ptr [d + offset]);
		xor_(a, a);
		for (size_t i = 0; i < pd.size(); i++) {
			mulx(hi, a, ptr [py + i * 8]);
			adox(pd[i], a);
			if (i == 0) mov(ptr[pz + offset], pd[0]);
			if (i == pd.size() - 1) break;
			adcx(pd[i + 1], hi);
		}
		mov(a, 0);
		adox(hi, a);
		adc(hi, a);
	}
	/*
		z[] = x[]
	*/
	void mov_rr(const Pack& z, const Pack& x)
	{
		assert(z.size() == x.size());
		for (int i = 0, n = (int)x.size(); i < n; i++) {
			mov(z[i], x[i]);
		}
	}
	/*
		m[] = x[]
	*/
	void store_mr(const RegExp& m, const Pack& x)
	{
		for (int i = 0, n = (int)x.size(); i < n; i++) {
			mov(ptr [m + 8 * i], x[i]);
		}
	}
	void store_mr(const Xbyak::RegRip& m, const Pack& x)
	{
		for (int i = 0, n = (int)x.size(); i < n; i++) {
			mov(ptr [m + 8 * i], x[i]);
		}
	}
	/*
		x[] = m[]
	*/
	template<class ADDR>
	void load_rm(const Pack& z, const ADDR& m)
	{
		for (int i = 0, n = (int)z.size(); i < n; i++) {
			mov(z[i], ptr [m + 8 * i]);
		}
	}
	/*
		z[] += x[]
	*/
	void add_rr(const Pack& z, const Pack& x)
	{
		add(z[0], x[0]);
		assert(z.size() == x.size());
		for (size_t i = 1, n = z.size(); i < n; i++) {
			adc(z[i], x[i]);
		}
	}
	/*
		z[] -= x[]
	*/
	void sub_rr(const Pack& z, const Pack& x)
	{
		sub(z[0], x[0]);
		assert(z.size() == x.size());
		for (size_t i = 1, n = z.size(); i < n; i++) {
			sbb(z[i], x[i]);
		}
	}
	/*
		z[] += m[]
	*/
	template<class ADDR>
	void add_rm(const Pack& z, const ADDR& m, bool withCarry = false)
	{
		if (withCarry) {
			adc(z[0], ptr [m + 8 * 0]);
		} else {
			add(z[0], ptr [m + 8 * 0]);
		}
		for (int i = 1, n = (int)z.size(); i < n; i++) {
			adc(z[i], ptr [m + 8 * i]);
		}
	}
	/*
		z[] -= m[]
	*/
	template<class ADDR>
	void sub_rm(const Pack& z, const ADDR& m, bool withCarry = false)
	{
		if (withCarry) {
			sbb(z[0], ptr [m + 8 * 0]);
		} else {
			sub(z[0], ptr [m + 8 * 0]);
		}
		for (int i = 1, n = (int)z.size(); i < n; i++) {
			sbb(z[i], ptr [m + 8 * i]);
		}
	}
	void cmovc_rr(const Pack& z, const Pack& x)
	{
		for (int i = 0, n = (int)z.size(); i < n; i++) {
			cmovc(z[i], x[i]);
		}
	}
	// y[i] &= t
	void and_pr(const Pack& y, const Reg64& t)
	{
		for (int i = 0; i < (int)y.size(); i++) {
			and_(y[i], t);
		}
	}
};

