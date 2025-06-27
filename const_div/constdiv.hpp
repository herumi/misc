#include <stdint.h>
static const uint64_t one = 1;
static const uint32_t N = 32;
static const uint64_t M = (one << N) - 1;

inline uint32_t floor_ilog2(uint32_t x)
{
	uint32_t a = 0;
	while ((one << a) <= x) a++;
	return a - 1;
}

/*
M: integer >= 1.
d in [1, M]
take a. A := 2^a >= d. c := (A + d - 1) // d.
e = d c - A. 0 <= e <= d-1 < A.
(q0, r0) = divmod(M, d). M = q0 d + r0.
M0 := M - ((M+1)%d). Then M0%d = d-1.
Theorem
if e M0 < A, then (x c)//A = x//d. for x in [0, M].
Proof
(q, r) := divmod(x, d). x = d q + r.
Then x c = d q c + r c = (A + e) q + r c = A q + (q e + r c).
y := q e + r c.
If 0 <= y < A, then q = (x c) >> a.
So we prove that y < A if and only if e M0 < A.
y d = q e d + r c d = e q d + r (e + A) = e(d q + r) + r A = e x + r A.
Consider max(y d).
if r0 = d-1, then max(y d) (at x=M) = e M + (d-1) A = e M0 + (d-1) A.
if r0 < d-1, then max(y d) (at x=M0) = e M0 + (d-1) A because e < A.
Then max (y d) = e M0 + (d-1)A.
max(y) < A iff max(y d) < d A iff e M0 + (d-1) A < d A iff e M0 < A.
e M0 < A iff c/A < (1 + 1/M0)/d.
This condition is the assumption of Thereom 1 in
"Integer division by constants: optimal bounds", Daniel Lemire, Colin Bartlett, Owen Kaser. 2021
*/
struct ConstDiv {
	uint32_t d_;
	uint32_t a_;
	uint64_t A_;
	uint64_t c_;
	ConstDiv() : d_(0), a_(0), A_(0), c_(0) {}
	void put() const
	{
		printf("My d=%u(0x%08x) a=%u u=0x%" PRIx64 "\n", d_, d_, a_, c_);
	}
	bool init(uint32_t d)
	{
		assert(d <= M);
		uint32_t M0 = M - ((M+1)%d);
		// u > 0 => A >= d => a >= ilog2(d)
		for (uint32_t a = floor_ilog2(d); a < 64; a++) {
			uint64_t A = one << a;
			uint64_t c = (A + d - 1) / d;
			assert(c < (one << 33));
			if (c >= (one << 33)) continue; // same result if this line is comment out.
			uint64_t e = d * c - A;
			assert(e < A);
			if (e * M0 < A) {
				assert(e < c);
				d_ = d;
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

#if defined(_WIN64) || defined(__x86_64__)
#define CONST_DIV_GEN

#define XBYAK_DISABLE_AVX512
#include <xbyak/xbyak_util.h>

typedef uint32_t (*DivFunc)(uint32_t);
typedef uint64_t (*DivFuncRet64)(uint32_t);

static const size_t FUNC_N = 1 + 4;

struct ConstDivGen : Xbyak::CodeGenerator {
	DivFunc divd;
	DivFunc divLp[FUNC_N];
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
	void divRaw(const ConstDiv& cd, int mode, const Xbyak::Reg32& x)
	{
		if (d_ >= 0x80000000) {
			xor_(eax, eax);
			cmp(x, d_);
			setae(al);
			return;
		}
		if (cd.c_ <= 0xffffffff) {
			mov(eax, x);
			if (cd.c_ > 1) {
				mov(edx, cd.c_);
				mul(rdx);
			}
			shr(rax, cd.a_);
			return;
		}
		if (mode < 0) {
			// generated asm code by gcc/clang
			mov(edx, x);
			mov(eax, edx);
			imul(rax, rax, cd.c_ & 0xffffffff);
			shr(rax, 32);
			sub(edx, eax);
			shr(edx, 1);
			add(eax, edx);
			shr(eax, cd.a_ - 33);
			return;
		}
		mov(eax, x);
		mov(rdx, cd.c_);
		if (mode & (1<<1)) {
			mulx(rdx, rax, rax);
		} else {
			mul(rdx);
		}
		if (mode & (1<<2)) {
			shrd(rax, rdx, cd.a_);
		} else {
			shr(rax, cd.a_);
			shl(edx, 64 - cd.a_);
			or_(eax, edx);
		}
	}
	bool init(uint32_t d, int mode = -1)
	{
		using namespace Xbyak;
		using namespace Xbyak::util;

		ConstDiv cd;
		if (!cd.init(d)) return false;
		d_ = cd.d_;
		a_ = cd.a_;
		{
			align(32);
			divd = getCurr<DivFunc>();
			StackFrame sf(this, 1, UseRDX);
			const Reg32 x = sf.p[0].cvt32();
			divRaw(cd, mode, x);
		}
		for (size_t i = 0; i < FUNC_N; i++) {
			align(32);
			divLp[i] = getCurr<DivFunc>();
			StackFrame sf(this, 1, 2|UseRDX);
			const Reg32 n = sf.p[0].cvt32();
			const Reg64& sum = sf.t[0];
			const Reg32 x = sf.t[1].cvt32();
			xor_(sum, sum);
			xor_(x, x);
			align(32);
			Label lpL;
			L(lpL);
			divRaw(cd, i, x);
			add(sum, rax);
			add(x, 1);
			dec(n);
			jnz(lpL);
			mov(rax, sum);
		}
		setProtectModeRE();
		return true;
	}
	void dump() const
	{
		FILE *fp = fopen("bin", "wb");
		if (fp) {
			fwrite(getCode(), 1, getSize(), fp);
			fclose(fp);
		}
	}
	void put() const
	{
		printf("Gen d=%u(0x%08x) a=%u divd=%p\n", d_, d_, a_, divd);
	}
};

#endif

