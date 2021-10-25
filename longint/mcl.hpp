#pragma once

#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

#include <stdio.h>
#include <assert.h>
#include "mcl.h"

using namespace Xbyak;
using namespace Xbyak::util;

typedef uint64_t Unit;

static const char *pStr11 = "0x9401ff90f28bffb0c610fb10bf9e0fefd59211629a7991563c5e468d43ec9cfe1549fd59c20ab5b9a7cda7f27a0067b8303eeb4b31555cf4f24050ed155555cd7fa7a5f8aaaaaaad47ede1a6aaaaaaaab69e6dcb";

// dummy
static const char *pStr9 = "0x10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000201";

void3u mcl_mulPre;
void3u mcl_mont;

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

void dump(const char *msg, const Pack& p)
{
	printf("%s ", msg);
	for (size_t i = 0; i < p.size(); i++) {
		printf("%s ", p[i].toString());
	}
	printf("\n");
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
	int N;
	Unit rp_;
	Unit p_[11];
	int bitSize;

	/*
		@param op [in] ; use op.p, op.N, op.isFullBit
	*/
	Code()
		: CodeGenerator(4096 * 9)
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
		, N(0)
		, rp_(0)
		, p_()
	{
	}
	void init(int n)
	{
		N = n;
		if (N != 9 && N != 11) throw cybozu::Exception("mcl::init not uspport n") << n;
		mpz_class p(N == 11 ? pStr11 : pStr9);
		bitSize = mcl::gmp::getBitSize(p);
		mcl::gmp::getArray(p_, n, p);
		rp_ = getMontgomeryCoeff(p_[0]);
		printf("bitSize=%d rp_=%016llx\n", bitSize, (long long)rp_);

		align(16);
		mcl_mulPre = getCurr<void3u>();
		if (N == 11) {
			gen_mulPre11();
		} else {
			gen_mulPre9();
		}

		align(16);
		mcl_mont = getCurr<void3u>();
		gen_montMul11();
	}
private:
	Code(const Code&);
	void operator=(const Code&);
	// [gp0] <- [gp1] * [gp2]
	void gen_mulPre11()
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

		Pack pk(ta, t9, t8, t7, t6, t5, t4, t3, t2, t1, t0);
		mulPack(pz, xm0, 8 * 0, py, pk);
		Reg64 t = tb;
		for (int i = 1; i < 11; i++) {
			mulPackAdd(pz, xm0, 8 * i, py, t, pk);
			Reg64 s = pk[0];
			pk = pk.sub(1);
			pk.append(t);
			t = s;
		}
		store_mr(pz + 8 * 11, pk);
		movq(rsp, xm1);
	}
	/*
		[pd:pz[0]] <- py[n-1..0] * px[0]
		use xmm
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
		[pd:pz[0]] <- py[n-1..0] * px[0]
		does not use xmm
	*/
	void mulPack(const Reg64& pz, const Reg64& px, int offset, const RegExp& py, const Pack& pd)
	{
		const Reg64& a = rax;
		const Reg64& d = rdx;
		mov(d, ptr [px + offset]);
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
		[hi:Pack(d_(n-1), .., d1):pz[0]] <- Pack(d_(n-1), ..., d0) + py[n-1..0] * px[0]
	*/
	void mulPackAdd(const RegExp& pz, const Reg64& px, int offset, const RegExp& py, const Reg64& hi, const Pack& pd)
	{
		const Reg64& a = rax;
		const Reg64& d = rdx;
		mov(d, ptr [px + offset]);
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
	Pack rotatePack(const Pack& p) const
	{
		Pack q = p.sub(1);
		q.append(p[0]);
		return q;
	}
	// use xm0, .., xm4
	void gen_montMul11()
	{
		StackFrame sf(this, 3, 10 | UseRDX);//, 0, false);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		Pack pk = sf.t;
		pk.append(py);
		pk.append(rax);
		assert(pk.size() == N + 1);

		Label exitL;
		mov(rax, size_t(p_));
		movq(xm1, px);
		movq(xm2, rax);
		movq(xm3, py);
		movq(xm4, pz);
		for (int i = 0; i < N; i++) {
			movq(rdx, xm3);
			mov(rdx, ptr [rdx + i * 8]);
			montgomery11_1(pk, xm1, xm2, px, pz, xm0, i == 0);
			if (i < N - 1) pk = rotatePack(pk);
		}
		pk = pk.sub(1);

		movq(pz, xm4);
		store_mr(pz, pk);
		movq(py, xm2); // p
		sub_rm(pk, py); // z - p
		jc(exitL);
		store_mr(pz, pk);
	L(exitL);
	}
	/*
		c[n..0] = px[n-1..0] * rdx
		use t
	*/
	void mulPack1(const Pack& c, const Reg64& px, const Reg64& t)
	{
		const int n = c.size() - 1;
		mulx(c[1], c[0], ptr [px + 0 * 8]);
		for (int i = 1; i < n; i++) {
			mulx(c[i + 1], t, ptr[px + i * 8]);
			if (i == 1) {
				add(c[i], t);
			} else {
				adc(c[i], t);
			}
		}
		adc(c[n], 0);
	}
	/*
		input
		c[n-1..0]
		rdx = yi
		use rax, rdx
		output
		c[n..1]

		if first:
		  c = x[n-1..0] * rdx
		else:
		  c += x[n-1..0] * rdx
		q = uint64_t(c0 * rp)
		c += p * q
		c >>= 64
	*/
	void montgomery11_1(const Pack& c, const Xmm& xpx, const Xmm& xpp, const Reg64& t0, const Reg64& t1, const Xmm& xt, bool isFirst)
	{
		const Reg64& d = rdx;
		movq(t0, xpx);
		if (isFirst) {
			// c[n..0] = px[n-1..0] * rdx
			mulPack1(c, t0, t1);
		} else {
			// c[n..0] = c[n-1..0] + px[n-1..0] * rdx because of not fuill bit
			mulAdd(c, t0, t1, xt, true);
		}
		mov(d, rp_);
		imul(d, c[0]); // d = q = uint64_t(d * c[0])
		// c[n..0] += p * q because of not fuill bit
		movq(t0, xpp);
		mulAdd(c, t0, t1, xt, false);
	}
	// [gp0] <- [gp1] * [gp2]
	void gen_mulPre9()
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

		Pack pk(t8, t7, t6, t5, t4, t3, t2, t1, t0);
		mulPack(pz, px, 9 * 0, py, pk);
		Reg64 t = sf.t[9];
		for (int i = 1; i < 9; i++) {
			mulPackAdd(pz, px, 8 * i, py, t, pk);
			Reg64 s = pk[0];
			pk = pk.sub(1);
			pk.append(t);
			t = s;
		}
		store_mr(pz + 8 * 9, pk);
	}
	/*
		c[n..0] = c[n-1..0] + px[n-1..0] * rdx if is_cn_zero = true
		c[n..0] = c[n..0] + px[n-1..0] * rdx if is_cn_zero = false
		use rax, rdx, t0, t1
	*/
	void mulAdd(const Pack& c, const Reg64& px, const Reg64& t, const Xmm& xt, bool is_cn_zero)
	{
		const int n = c.size() - 1;
		if (is_cn_zero) {
			xor_(c[n], c[n]);
		} else {
			xor_(t, t); // clear ZF
		}
		movq(xt, c[n]); // save
		for (int i = 0; i < n; i++) {
			mulx(t, c[n], ptr [px + i * 8]);
			adox(c[i], c[n]);
			if (i == n - 1) break;
			adcx(c[i + 1], t);
		}
		movq(c[n], xt);
		adox(c[n], t);
		adc(c[n], 0);
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
	template<class REGT>
	void store_mr(const REGT& m, const Pack& x)
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

