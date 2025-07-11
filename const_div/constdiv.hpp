#include <stdint.h>
#include <fstream>
#include "uint128_t.hpp"

/*
M: integer >= 1.
d in [1, M]
take A >= d.
  c := (A + d - 1) // d.
  e := d c - A.
  M_d := M - ((M+1)%d).

Lemma.
1. By definition,
  0 <= e <= d-1 < A.
  M_d%d = d-1.
  d-1 <= M_d.
2. if e M_d < A, then e(d-1) < A = d c - e, so e < c.

Theorem
if e M_d < A, then x//d = (x c)//A for x in [0, M].

Proof
(q, r) := divmod(x, d). x = d q + r.
Then x c = d q c + r c = (A + e) q + r c = A q + (q e + r c).
y := q e + r c.
If 0 <= y < A, then q = (x c) // A.
So we prove that y < A if e M_d < A.
f(x) := dy = d q e + d r c = d q e + (A + e) r = (d q + r) e + r A = x e + r A.
So if max f(x) < d A, then max(y) = max f(x) / d < A.

e and A are constant values and >= 0, then arg_max f(x) = M_d or M.
i.e., (x, r) = (M_d, d-1) or (M, r0) where r0 := M % d.

Claim max f(x) = f(M_d) < d A.
case 1. M_d = M. then r0 = d-1. max f(x) = f(M_d) = M_d e + (d-1) A < A + (d-1) A = d A.
case 2. M_d < M. then r0 < d-1. max f(x) = max(f(M_d), f(M)).
M = M_d + 1 + r0.
f(M_d) - f(M) = (M_d e + (d-1)A) - (M e + r0 A) = (d-1 - r0) A - (r0 + 1)e
>(d-1 - (d-2)) A - ((d-2)+1)e = A - d e + e = d (c - e) > 0.
Then max f(x) = f(M_d) < d A.

This condition is the assumption of Thereom 1 in
"Integer division by constants: optimal bounds", Daniel Lemire, Colin Bartlett, Owen Kaser. 2021
*/
struct ConstDiv {
	static const uint64_t one = 1;
	static const uint32_t N = 32;
	static const uint64_t M = (one << N) - 1;

	static inline uint32_t ceil_ilog2(uint32_t x)
	{
		uint32_t a = 0;
		while ((one << a) <= x) a++;
		return a - 1;
	}

	uint32_t d_;
	uint32_t a_;
	uint64_t A_;
	uint64_t c_;
	bool cmp_; // use comparison
	ConstDiv() : d_(0), a_(0), A_(0), c_(0), cmp_(false) {}
	void put() const
	{
		printf("My d=%u(0x%08x) a=%u c32=0x%08x over=%d cmp=%d\n", d_, d_, a_, uint32_t(c_), c_ >= 0x100000000, cmp_);
	}
	bool init(uint32_t d)
	{
		d_ = d;
		assert(d <= M);
		const uint32_t M_d = M - ((M+1)%d);
		if (d > 0x80000000) {
			cmp_ = true;
			uint32_t a = 64;
			uint64_t c = 0xffffffffffffffff / d + 1;
			if (c <= 0xffffffff || c >= (one << 33)) {
				return false;
			}
			a_ = a;
			A_ = 0;
			c_ = c;
			return true;
		}
		// u > 0 => A >= d => a >= ilog2(d)
		for (uint32_t a = ceil_ilog2(d); a < 64; a++) {
			uint64_t A = one << a;
			uint64_t c = (A + d - 1) / d;
			assert(c < (one << 33));
			if (c >= (one << 33)) return false;
			uint64_t e = d * c - A;
			if (e * M_d < A) {
				a_ = a;
				A_ = A;
				c_ = c;
				return true;
			}
		}
		return false;
	}
	uint32_t divd(uint32_t x) const
	{
		if (cmp_) {
#if 0 // #ifdef MCL_DEFINED_UINT128_T
			uint128_t v = (x * uint128_t(c_)) >> a_;
			return uint32_t(v);
#else
			return x >= d_;
#endif
		}
		if (c_ > 0xffffffff) {
#ifdef MCL_DEFINED_UINT128_T
			uint128_t v = (x * uint128_t(c_)) >> a_;
			return uint32_t(v);
#else
#if 1
			uint64_t v = x * (c_ & 0xffffffff);
			v >>= 32;
			v += x;
			v >>= a_-32;
			return uint32_t(v);
#else
			uint64_t H;
			uint64_t L = mulUnit1(&H, x, c);
			L >>= a;
			H <<= (64 - a);
			return uint32_t(H | L);
#endif
#endif
		} else {
			uint32_t v = uint32_t((x * c_) >> a_);
			return v;
		}
	}
};

typedef uint32_t (*DivFunc)(uint32_t);

#if defined(_WIN64) || defined(__x86_64__)
#define CONST_DIV_GEN

#define XBYAK_DISABLE_AVX512
#include <xbyak/xbyak_util.h>

static const size_t FUNC_N = 1 + 5;

struct ConstDivGen : Xbyak::CodeGenerator {
	DivFunc divd;
	DivFunc divLp[FUNC_N];
	const char *name[FUNC_N];
	uint32_t d_;
	uint32_t a_;
	ConstDivGen()
		: Xbyak::CodeGenerator(4096, Xbyak::DontSetProtectRWE)
		, divd(nullptr)
		, divLp{}
		, d_(0)
		, a_(0)
	{
	}
	// eax = x/d
	// use rax, rdx
	void divRaw(const ConstDiv& cd, uint32_t mode, const Xbyak::Reg32& x)
	{
		if (d_ >= 0x80000000) {
			name[mode] = "cmp";
			xor_(eax, eax);
			cmp(x, d_);
			setae(al);
			return;
		}
		if (cd.c_ <= 0xffffffff) {
			mov(eax, x);
			if (cd.c_ > 1) {
				name[mode] = "mul+shr";
				mov(edx, cd.c_);
				mul(rdx);
			} else {
				name[mode] = "shr";
			}
			shr(rax, cd.a_);
			return;
		}
		if (mode == FUNC_N-1) {
			name[FUNC_N-1] = "gcc";
			// generated asm code by gcc/clang
			mov(edx, x);
			mov(eax, cd.c_ & 0xffffffff);
			imul(rax, rdx);
			shr(rax, 32);
			sub(edx, eax);
			shr(edx, 1);
			add(eax, edx);
			shr(eax, cd.a_ - 33);
			return;
		}
		if (mode == FUNC_N-2) {
			name[FUNC_N-2] = "my";
			mov(eax, cd.c_ & 0xffffffff);
			imul(rax, x.cvt64());
			shr(rax, 32);
			add(rax, x.cvt64());
			shr(rax, cd.a_ - 32);
			return;
		}
		mov(eax, x);
		mov(rdx, cd.c_);
		static const char *nameTbl[] = {
			"mul/mixed",
			"mulx/mixed",
			"mul/shrd",
			"mulx/shrd",
		};
		name[mode] = nameTbl[mode];
		if (mode & (1<<1)) {
			mulx(rdx, rax, rax);
		} else {
			mul(rdx);
		}
		if (mode & (1<<2)) {
			shrd(rax, rdx, uint8_t(cd.a_));
		} else {
			shr(rax, cd.a_);
			shl(edx, 64 - cd.a_);
			or_(eax, edx);
		}
	}
	bool init(uint32_t d, uint32_t lpN)
	{
		using namespace Xbyak;
		using namespace Xbyak::util;

		d_ = d;
		ConstDiv cd;
		if (!cd.init(d)) return false;
		cd.put();
		a_ = cd.a_;
		{
			align(32);
			divd = getCurr<DivFunc>();
			StackFrame sf(this, 1, UseRDX);
			const Reg32 x = sf.p[0].cvt32();
			divRaw(cd, FUNC_N-2, x);
		}
		for (uint32_t i = 0; i < FUNC_N; i++) {
			align(32);
			divLp[i] = getCurr<DivFunc>();
			StackFrame sf(this, 1, 2|UseRDX);
			const Reg32 n = sf.p[0].cvt32();
			const Reg32 sum = sf.t[0].cvt32();
			const Reg32 x = sf.t[1].cvt32();
			xor_(sum, sum);
			xor_(x, x);
			align(32);
			Label lpL;
			L(lpL);
			for (uint32_t j = 0; j < lpN; j++) {
				divRaw(cd, i, x);
				add(sum, eax);
			}
			add(x, 1);
			dec(n);
			jnz(lpL);
			mov(eax, sum);
		}
		setProtectModeRE();
		return true;
	}
	void dump() const
	{
		std::ofstream ofs("bin", std::ios::binary);
		ofs.write(reinterpret_cast<const char*>(getCode()), getSize());
	}
	void put() const
	{
		printf("Gen d=%u(0x%08x) a=%u divd=%p\n", d_, d_, a_, divd);
	}
};

#elif defined(__arm64__) || defined(__aarch64__)

#define CONST_DIV_GEN

#include <xbyak_aarch64/xbyak_aarch64.h>

static const size_t FUNC_N = 3;

struct ConstDivGen : Xbyak_aarch64::CodeGenerator {
	DivFunc divd;
	DivFunc divLp[FUNC_N];
	const char *name[FUNC_N];
	uint32_t d_;
	uint32_t a_;
	ConstDivGen()
		: divd(nullptr)
		, divLp{}
		, d_(0)
		, a_(0)
	{
	}

	// input x
	// output x = x/d
	// use w9, w10
	void divRaw(const ConstDiv& cd, uint32_t mode, const Xbyak_aarch64::WReg& wx)
	{
		using namespace Xbyak_aarch64;
		const XReg x = XReg(wx.getIdx());
		if (d_ >= 0x80000000) {
			name[mode] = "cmp";
			uint32_t dL = uint32_t(d_ & 0xffff);
			uint32_t dH = uint32_t(d_ >> 16);
			mov(w9, dL);
			movk(w9, dH, 16);
			cmp(x, x9);
			cset(x, HS); // Higher or Same
			return;
		}
		uint32_t cL = uint32_t(cd.c_ & 0xffff);
		uint32_t cH = uint32_t((cd.c_ >> 16) & 0xffff);
		if (cd.c_ > 1) {
			mov(w9, cL);
			movk(w9, cH, 16);
		}
		if (cd.c_ <= 0xffffffff) {
			if (cd.c_ > 1) {
				name[mode] = "mul+shr";
				umull(x, wx, w9);
			} else {
				name[mode] = "shr";
			}
			lsr(x, x, cd.a_);
			return;
		}
		switch (mode) {
		case 0:
			name[0] = "my";
			umull(x9, wx, w9); // x9 = [cH:cL] * x
			add(x, x, x9, LSR, 32); // x += x9 >> 32;
			lsr(x, x, cd.a_ - 32);
			return;
		case 1:
			name[1] = "mul64";
			movk(x9, 1, 32); // x9 = c = [1:cH:cL]
			mul(x10, x9, x);
			umulh(x9, x9, x); // [x9:x10] = c * x
			extr(x, x9, x10, cd.a_); // x = [x9:x10] >> a_
			return;
		default:
			name[2] = "clang";
			// same code generated by clang
			umull(x9, wx, w9);
			lsr(x9, x9, 32);
			sub(w10, wx, w9);
			add(w9, w9, w10, LSR, 1);
			lsr(wx, w9, cd.a_ - 33);
			return;
		}
	}
	bool init(uint32_t d, uint32_t lpN)
	{
		using namespace Xbyak_aarch64;
		ConstDiv cd;
		if (!cd.init(d)) return false;
		cd.put();
		d_ = cd.d_;
		a_ = cd.a_;
		{
			align(32);
			divd = getCurr<DivFunc>();
			divRaw(cd, 0, w0);
			ret();
		}
		for (uint32_t mode = 0; mode < FUNC_N; mode++) {
			align(32);
			divLp[mode] = getCurr<DivFunc>();
			const auto n = w0;
			const auto sum = w1;
			const auto x = w2;
			const auto i = w3;
			mov(sum, 0);
			mov(i, 0);
			Label lpL;
			L(lpL);
			for (uint32_t j = 0; j < lpN; j++) {
				mov(x, i);
				divRaw(cd, mode, x);
				add(sum, sum, x);
			}
			add(i, i, 1);
			subs(n, n, 1);
			bne(lpL);
			mov(w0, sum);
			ret();
		}
		ready();
		return true;
	}
	void dump() const
	{
		std::ofstream ofs("bin", std::ios::binary);
		ofs.write(reinterpret_cast<const char*>(getCode()), getSize());
	}
	void put() const
	{
		printf("Gen d=%u(0x%08x) a=%u divd=%p\n", d_, d_, a_, divd);
	}
};

#endif

