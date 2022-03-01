#pragma once

#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

#include <stdio.h>
#include <assert.h>
#include "mcl.h"

using namespace Xbyak;
using namespace Xbyak::util;

typedef uint64_t Unit;

void3u mcl_mulPre;
void3u mcl_mont;
void2u mcl_mod;
void3u mcl_add;
void3u mcl_sub;
void2u mcl_neg;
void2u mcl_mul2;

void3u mcl_addDbl;
void3u mcl_subDbl;
void2u mcl_negDbl;
void2u mcl_mul2Dbl;

void3u mcl_addPre;
void3u mcl_subPre;
void3u mcl_addDblPre;
void3u mcl_subDblPre;

template<class T>
void setFuncInfo(Xbyak::util::Profiler& prof, const char *name, const T& begin, const uint8_t* end)
{
	const uint8_t*p = (const uint8_t*)begin;
	prof.set((std::string("mcl_") + name).c_str(), p, end - p);
}

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
	static const int MAX_N = 11;
	int N;
	Unit rp_;
	Unit p_[MAX_N];
	int bitSize;
	Xbyak::util::Profiler prof_;

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
	void init(const char *pStr)
	{
		const char *s = getenv("MCL_PERF");
		int profMode = 0;
		if (s && s[0] && s[1] == '\0') profMode = s[0] - '0';
		printf("profMode=%d\n", profMode);
		if (profMode) {
			prof_.init(profMode);
			prof_.setStartAddr(getCurr());
		}
		mpz_class p(pStr);
		bitSize = mcl::gmp::getBitSize(p);
		N = (bitSize + 63) / 64;
		if (N > MAX_N) throw cybozu::Exception("too large pStr") << N;
		if (N != 9 && N != 11) throw cybozu::Exception("not support N") << N;
		mcl::gmp::getArray(p_, N, p);
		rp_ = getMontgomeryCoeff(p_[0]);
		printf("bitSize=%d rp_=%016llx\n", bitSize, (long long)rp_);

		gen_mulPre(N);
		gen_montMul(N);
		gen_mod(N);
		mcl_add = getCurr<void3u>();
		gen_add(N);
		setFuncInfo(prof_, "_add", mcl_add, getCurr());
		mcl_sub = getCurr<void3u>();
		gen_sub(N);
		setFuncInfo(prof_, "_sub", mcl_sub, getCurr());
		mcl_neg = getCurr<void2u>();
		gen_neg(N);
		setFuncInfo(prof_, "_neg", mcl_neg, getCurr());
		mcl_mul2 = getCurr<void2u>();
		gen_mul2(N);
		setFuncInfo(prof_, "_mul2", mcl_mul2, getCurr());

		mcl_addDbl = getCurr<void3u>();
		gen_addDbl(N);
		setFuncInfo(prof_, "_addDbl", mcl_addDbl, getCurr());
		mcl_subDbl = getCurr<void3u>();
		gen_subDbl(N);
		setFuncInfo(prof_, "_subDbl", mcl_subDbl, getCurr());
		mcl_negDbl = getCurr<void2u>();
		gen_negDbl(N);
		setFuncInfo(prof_, "_negDbl", mcl_negDbl, getCurr());
		mcl_mul2Dbl = getCurr<void2u>();
		gen_mul2Dbl(N);
		setFuncInfo(prof_, "_mul2Dbl", mcl_mul2Dbl, getCurr());

		mcl_addPre = getCurr<void3u>();
		gen_addPre(N);
		setFuncInfo(prof_, "_addPre", mcl_addPre, getCurr());
		mcl_subPre = getCurr<void3u>();
		gen_subPre(N);
		setFuncInfo(prof_, "_subPre", mcl_subPre, getCurr());
		mcl_addDblPre = getCurr<void3u>();
		gen_addPre(N * 2);
		setFuncInfo(prof_, "_addDblPre", mcl_addDblPre, getCurr());
		mcl_subDblPre = getCurr<void3u>();
		gen_subPre(N * 2);
		setFuncInfo(prof_, "_subDblPre", mcl_subDblPre, getCurr());
	}
private:
	Code(const Code&);
	void operator=(const Code&);

	// addr = t if < p else t-p
	// use rax
	template<class ADDR>
	void store_modp(const ADDR& addr, const Pack& t)
	{
		Label exitL;
		mov(rax, size_t(p_));
		cmp(t[N-1], ptr[rax + (N-1)*8]); // shortcut
		jb(exitL, T_NEAR);
		sub_rm(t, rax);
		jc(exitL);
		store_mr(addr, t);
	L(exitL);
	}
	void gen_add(size_t n)
	{
		StackFrame sf(this, 3, n);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		Pack t = sf.t;
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			if (i == 0) {
				add(t[i], ptr[py + i * 8]);
			} else {
				adc(t[i], ptr[py + i * 8]);
			}
			mov(ptr[pz + i * 8], t[i]);
		}
		store_modp(pz, t);
	}
	void gen_sub(size_t n)
	{
		StackFrame sf(this, 3, n);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		Pack t = sf.t;
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			if (i == 0) {
				sub(t[i], ptr[py + i * 8]);
			} else {
				sbb(t[i], ptr[py + i * 8]);
			}
		}
		Label exitL;
		jnc(exitL);
		mov(rax, size_t(p_));
		add_rm(t, rax);
	L(exitL);
		store_mr(pz, t);
	}
	void gen_addPre(size_t n)
	{
		StackFrame sf(this, 3);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		for (size_t i = 0; i < n; i++) {
			mov(rax, ptr[px + i * 8]);
			if (i == 0) {
				add(rax, ptr[py + i * 8]);
			} else {
				adc(rax, ptr[py + i * 8]);
			}
			mov(ptr[pz + i * 8], rax);
		}
	}
	void gen_subPre(size_t n)
	{
		StackFrame sf(this, 3);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		for (size_t i = 0; i < n; i++) {
			mov(rax, ptr[px + i * 8]);
			if (i == 0) {
				sub(rax, ptr[py + i * 8]);
			} else {
				sbb(rax, ptr[py + i * 8]);
			}
			mov(ptr[pz + i * 8], rax);
		}
	}
	void gen_neg(size_t n)
	{
		StackFrame sf(this, 3, n);
		const Reg64& py = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& v = sf.p[2];
		Pack t = sf.t;
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			if (i == 0) {
				mov(rax, t[0]);
			} else {
				or_(rax, t[i]);
			}
		}
		test(rax, rax);
		Label nonZeroL, exitL;
		jnz(nonZeroL, T_NEAR);
		store_mr(py, t);
		jmp(exitL, T_NEAR);
	L(nonZeroL);
		mov(rax, size_t(p_));
		for (size_t i = 0; i < n; i++) {
			mov(v, ptr[rax + i * 8]);
			if (i == 0) {
				sub(v, t[i]);
			} else {
				sbb(v, t[i]);
			}
			mov(ptr[py + i * 8], v);
		}
	L(exitL);
	}
	void gen_mul2(size_t n)
	{
		StackFrame sf(this, 2, n);
		const Reg64& py = sf.p[0];
		const Reg64& px = sf.p[1];
		Pack t = sf.t;
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			if (i == 0) {
				add(t[i], t[i]);
			} else {
				adc(t[i], t[i]);
			}
			mov(ptr[py + i * 8], t[i]);
		}
		store_modp(py, t);
	}
	void gen_mulPreN(const Reg64& pz, const RegExp& px, const RegExp& py, Pack pk, Reg64 t)
	{
		mov(rdx, ptr[px + 8 * 0]);
		mulPack(pz, 8 * 0, py, pk);
		for (int i = 1; i < N; i++) {
			mov(rdx, ptr[px + 8 * i]);
			mulPackAdd(pz, 8 * i, py, t, pk);
			Reg64 s = pk[0];
			pk = pk.sub(1);
			pk.append(t);
			t = s;
		}
		store_mr(pz + 8 * N, pk);
	}

	void gen_mulPre(int n)
	{
		align(16);
		mcl_mulPre = getCurr<void3u>();
		switch (n) {
		case 9: gen_mulPre9(); break;
		case 11: gen_mulPre11(); break;
		default: throw cybozu::Exception("bad n") << n;
		}
		setFuncInfo(prof_, "_mulPre", mcl_mulPre, getCurr());
	}
	void gen_montMul(int n)
	{
		align(16);
		mcl_mont = getCurr<void3u>();
		switch (n) {
		case 9: gen_montMul9(); break;
		case 11: gen_montMul11(); break;
		default: throw cybozu::Exception("bad n") << n;
		}
		setFuncInfo(prof_, "_mont", mcl_mont, getCurr());
	}
	void gen_mod(int n)
	{
		align(16);
		mcl_mod = getCurr<void2u>();
		switch (n) {
		case 9: gen_mod9(); break;
		case 11: gen_mod11(); break;
		default: throw cybozu::Exception("bad n") << n;
		}
		setFuncInfo(prof_, "_mod", mcl_mod, getCurr());
	}
	// [gp0] <- [gp1] * [gp2]
	void gen_mulPre11()
	{
		StackFrame sf(this, 3, 10 | UseRDX, N * 8 * 2);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		// copy px and py to stack to free px and py
		for (int i = 0; i < N; i++) {
			mov(rax, ptr[px + 8 * i]);
			mov(ptr[rsp + 8 * i], rax);
			mov(rdx, ptr[py + 8 * i]);
			mov(ptr[rsp + 8 * N + 8 * i], rdx);
		}

		Pack pk = sf.t;
		pk.append(px);

		gen_mulPreN(pz, rsp, rsp + N * 8, pk, py);
	}
	/*
		input : rdx(implicit)
		[hi:Pack(d_(n-1), .., d1):pz[0]] <- Pack(d_(n-1), ..., d0) + py[n-1..0] * rdx
		use : rax
	*/
	void mulPackAdd(const RegExp& pz, int offset, const RegExp& py, const Reg64& hi, const Pack& pd)
	{
		const Reg64& a = rax;
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
		input : rdx(implicit)
		[pd:pz[offset:offset+n]] <- py[offset:offset+n] * rdx
		use : rax
	*/
	void mulPack(const Reg64& pz, int offset, const RegExp& py, const Pack& pd)
	{
		const Reg64& a = rax;
		mulx(pd[0], a, ptr [py + 8 * 0]);
		mov(ptr [pz + offset], a);
		xor_(a, a);
		for (size_t i = 1; i < pd.size(); i++) {
			mulx(pd[i], a, ptr [py + 8 * i]);
			adcx(pd[i - 1], a);
		}
		adc(pd[pd.size() - 1], 0);
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
		StackFrame sf(this, 3, 10 | UseRDX, N * 8 * 2);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		// copy px and py to stack to free px and py
#if 1
		for (int i = 0; i < N; i++) {
			mov(rax, ptr[px + 8 * i]);
			mov(ptr[rsp + 8 * i], rax);
			mov(rdx, ptr[py + 8 * i]);
			mov(ptr[rsp + 8 * N + 8 * i], rdx);
		}
#endif
		Pack pk = sf.t;
		pk.append(py);
		pk.append(rax);
		assert(pk.size() == N + 1);

		Label exitL;
		mov(px, size_t(p_));
		movq(xm0, pz);
		for (int i = 0; i < N; i++) {
			mov(rdx, ptr [rsp + i * 8]);
			montgomery11_1(pk, rsp + N * 8, px, pz, xm1, i == 0);
			if (i < N - 1) pk = rotatePack(pk);
		}
		pk = pk.sub(1);

		movq(pz, xm0);
		store_mr(pz, pk);
		sub_rm(pk, px); // z - p
		jc(exitL);
		store_mr(pz, pk);
	L(exitL);
	}
	/*
		c[n..0] = px[n-1..0] * rdx
		use t
	*/
	void mulPack1(const Pack& c, const RegExp& px, const Reg64& t)
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
	void montgomery11_1(const Pack& c, const RegExp& px, const RegExp& xp, const Reg64& t, const Xmm& xt, bool isFirst)
	{
		const Reg64& d = rdx;
		if (isFirst) {
			// c[n..0] = px[n-1..0] * rdx
			mulPack1(c, px, t);
		} else {
			// c[n..0] = c[n-1..0] + px[n-1..0] * rdx because of not fuill bit
			mulAdd(c, px, t, xt, true);
		}
		mov(d, rp_);
		imul(d, c[0]); // d = q = uint64_t(d * c[0])
		// c[n..0] += p * q because of not fuill bit
		mulAdd(c, xp, t, xt, false);
	}
	// [gp0] <- [gp1] * [gp2]
	void gen_mulPre9()
	{
		StackFrame sf(this, 3, 10 | UseRDX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];

		Pack pk = sf.t.sub(0, N);

		gen_mulPreN(pz, px, py, pk, sf.t[N]);
	}
	void gen_montMul9()
	{
		StackFrame sf(this, 3, 10 | UseRDX);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		Pack pk = sf.t;
		assert(pk.size() == N + 1);

		Label exitL;
		mov(rax, size_t(p_));
		movq(xm1, pz);
		for (int i = 0; i < N; i++) {
			mov(rdx, ptr [py + i * 8]);
			montgomery9_1(pk, px, rax, pz, xm0, i == 0);
			if (i < N - 1) pk = rotatePack(pk);
		}
		pk = pk.sub(1);

		movq(pz, xm1);
		store_mr(pz, pk);
		sub_rm(pk, rax); // z - p
		jc(exitL);
		store_mr(pz, pk);
	L(exitL);
	}
	void montgomery9_1(const Pack& c, const Reg64& px, const Reg64& pp, const Reg64& t1, const Xmm& xt, bool isFirst)
	{
		const Reg64& d = rdx;
		if (isFirst) {
			// c[n..0] = px[n-1..0] * rdx
			mulPack1(c, px, t1);
		} else {
			// c[n..0] = c[n-1..0] + px[n-1..0] * rdx because of not fuill bit
			mulAdd(c, px, t1, xt, true);
		}
		mov(d, rp_);
		imul(d, c[0]); // d = q = uint64_t(d * c[0])
		// c[n..0] += p * q because of not fuill bit
		mulAdd(c, pp, t1, xt, false);
	}
	/*
		c[n..0] = c[n-1..0] + px[n-1..0] * rdx if is_cn_zero = true
		c[n..0] = c[n..0] + px[n-1..0] * rdx if is_cn_zero = false
		use rdx, t, xt
	*/
	void mulAdd(const Pack& c, const RegExp& px, const Reg64& t, const Xmm& xt, bool is_cn_zero)
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
		@input (z, xy)
		z[n-1..0] <- montgomery reduction(x[2n-1..0])
	*/
	template<class ADDR>
	void gen_fpDbl_modNF(Pack pk, const Reg64& CF, const Reg64& tt, const ADDR& pp, const Xmm& tz, const RegExp& xy, int n, const Xmm *ta = 0)
	{
		assert(pk.size() == n + 1);
		const Reg64& d = rdx;

		xor_(CF, CF);
		load_rm(pk.sub(0, n), xy);
		mov(d, rp_);
		imul(d, pk[0]); // q
		mulAdd2(pk, xy + n * 8, pp, tt, CF, false, false, ta);

		for (int i = 1; i < n; i++) {
			pk.append(pk[0]);
			pk = pk.sub(1);
			mov(d, rp_);
			imul(d, pk[0]);
			mulAdd2(pk, xy + (n + i) * 8, pp, tt, CF, true, i < n - 1, ta);
		}

		pk = pk.sub(1);
		movq(tt, tz);
		store_mr(tt, pk);
		sub_rm(pk, pp);
		Label exitL;
		jc(exitL);
		store_mr(tt, pk);
	L(exitL);
	}
	void gen_mod9()
	{
		StackFrame sf(this, 3, 10 | UseRDX, 8 * N * 2);
		const Reg64& z = sf.p[0];
		const Reg64& pxy = sf.p[1];
		const Reg64& pp = sf.p[2];
		Pack pk = sf.t;
		movq(xm0, z);
		mov(pp, size_t(p_));
		// copy pxy to stack
		for (int i = 0; i < N * 2; i++) {
			mov(rax, ptr[pxy + 8 * i]);
			mov(ptr[rsp + 8 * i], rax);
		}
		gen_fpDbl_modNF(pk, pxy/*CF*/, z, pp, xm0, rsp, N);
	}
	void gen_mod11()
	{
		StackFrame sf(this, 3, 10 | UseRDX, 8 * N * 2, false);
		const Reg64& z = sf.p[0];
		const Reg64& pxy = sf.p[1];
		Pack pk = sf.t;
		pk.append(sf.p[2]);
		movq(xm0, z);
		Label ppL;
		// copy pxy to stack
		for (int i = 0; i < N * 2; i++) {
			mov(rax, ptr[pxy + 8 * i]);
			mov(ptr[rsp + 8 * i], rax);
		}
		pk.append(pxy);
		gen_fpDbl_modNF(pk, rax/*CF*/, z, rip + ppL, xm0, rsp, N, &xm1);
		sf.close();
		align(16);
	L(ppL);
		for (int i = 0; i < N; i++) {
			dq(p_[i]);
		}
	}
	/*
		output : CF:c[n..0] = c[n..0] + px[n-1..0] * rdx + (CF << n)
		inout : CF = 0 or 1
		use rax, tt
	*/
	template<class ADDR>
	void mulAdd2(const Pack& c, const RegExp& pxy, const ADDR& pp, const Reg64& tt, const Reg64& CF, bool addCF, bool updateCF = true, const Xmm *ta = 0)
	{
		assert(!isFullBit_);
		const Reg64& a = rax;
		assert(ta && CF == rax);
		if (ta) movq(*ta, CF);
		xor_(a, a);
		for (int i = 0; i < N; i++) {
			mulx(tt, a, ptr [pp + i * 8]);
			adox(c[i], a);
			if (i == 0) mov(c[N], ptr[pxy]);
			if (i == N - 1) break;
			adcx(c[i + 1], tt);
		}
		// we can suppose that c[0] = 0
		adox(tt, c[0]); // no carry
		if (ta) movq(CF, *ta);
		if (addCF) adox(tt, CF); // no carry
		adcx(c[N], tt);
		if (updateCF) setc(CF.cvt8());
	}
	void gen_addDbl(size_t n)
	{
		StackFrame sf(this, 3, n);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		Pack t = sf.t;
		// pz[0:n] = px[0:n] + py[0:n]
		for (size_t i = 0; i < n; i++) {
			mov(rax, ptr[px + i * 8]);
			if (i == 0) {
				add(rax, ptr[py + i * 8]);
			} else {
				adc(rax, ptr[py + i * 8]);
			}
			mov(ptr[pz + i * 8], rax);
		}
		lea(px, ptr[px + n * 8]);
		lea(py, ptr[py + n * 8]);
		lea(pz, ptr[pz + n * 8]);
		// pz[n:2n] = px[n:2n] + py[n:2n] mod p with CF
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			adc(t[i], ptr[py + i * 8]);
			mov(ptr[pz + i * 8], t[i]);
		}
		store_modp(pz, t);
	}
	void gen_subDbl(size_t n)
	{
		StackFrame sf(this, 3, n);
		const Reg64& pz = sf.p[0];
		const Reg64& px = sf.p[1];
		const Reg64& py = sf.p[2];
		Pack t = sf.t;
		// pz[0:n] = px[0:n] - py[0:n]
		for (size_t i = 0; i < n; i++) {
			mov(rax, ptr[px + i * 8]);
			if (i == 0) {
				sub(rax, ptr[py + i * 8]);
			} else {
				sbb(rax, ptr[py + i * 8]);
			}
			mov(ptr[pz + i * 8], rax);
		}
		lea(px, ptr[px + n * 8]);
		lea(py, ptr[py + n * 8]);
		lea(pz, ptr[pz + n * 8]);
		// pz[n:2n] = px[n:2n] - py[n:2n] mod p with CF
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			sbb(t[i], ptr[py + i * 8]);
		}
		Label exitL;
		jnc(exitL);
		mov(rax, size_t(p_));
		add_rm(t, rax);
	L(exitL);
		store_mr(pz, t);
	}
	void gen_negDbl(size_t n)
	{
		StackFrame sf(this, 2);
		const Reg64& py = sf.p[0];
		const Reg64& px = sf.p[1];
		Label nonZeroL, exitL;
		mov(rax, ptr[px]);
		test(rax, rax);
		jnz(nonZeroL, T_NEAR); // shortcut
		for (size_t i = 1; i < n * 2; i++) {
			or_(rax, ptr[px + i * 8]);
		}
		test(rax, rax);
		jnz(nonZeroL, T_NEAR);
		// zero clear
		for (size_t i = 0; i < n * 2; i++) {
			mov(ptr[py + i * 8], rax);
		}
		jmp(exitL, T_NEAR);
	L(nonZeroL);
		xor_(rax, rax);
		// lower : 0 - x
		for (size_t i = 0; i < n; i++) {
			mov(rdx, rax);
			if (i == 0) {
				sub(rdx, ptr[px + i * 8]);
			} else {
				sbb(rdx, ptr[px + i * 8]);
			}
			mov(ptr[py + i * 8], rdx);
		}
		// higher : p - x
		mov(rax, size_t(p_));
		lea(px, ptr[px + n * 8]);
		lea(py, ptr[py + n * 8]);
		for (size_t i = 0; i < n; i++) {
			mov(rdx, ptr[rax + i * 8]);
			sbb(rdx, ptr[px + i * 8]);
			mov(ptr[py + i * 8], rdx);
		}
	L(exitL);
	}
	void gen_mul2Dbl(size_t n)
	{
		StackFrame sf(this, 2, n);
		const Reg64& py = sf.p[0];
		const Reg64& px = sf.p[1];
		Pack t = sf.t;
		// py[0:n] = px[0:n]*2
		for (size_t i = 0; i < n; i++) {
			mov(rax, ptr[px + i * 8]);
			if (i == 0) {
				add(rax, rax);
			} else {
				adc(rax, rax);
			}
			mov(ptr[py + i * 8], rax);
		}
		lea(px, ptr[px + n * 8]);
		lea(py, ptr[py + n * 8]);
		// py[n:2n] = px[n:2n]*2 mod p with CF
		for (size_t i = 0; i < n; i++) {
			mov(t[i], ptr[px + i * 8]);
			adc(t[i], t[i]);
			mov(ptr[py + i * 8], t[i]);
		}
		store_modp(py, t);
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

