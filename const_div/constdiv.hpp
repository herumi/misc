#include <stdint.h>
#define XBYAK_DISABLE_AVX512
#include <xbyak/xbyak_util.h>

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
	uint32_t divp(uint32_t x) const
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

struct ConstDivGen : Xbyak::CodeGenerator {
	typedef uint32_t (*DivFunc)(uint32_t);
	DivFunc divp;
	uint32_t d_;
	uint32_t a_;
	ConstDivGen()
		: Xbyak::CodeGenerator(4096, Xbyak::DontSetProtectRWE)
		, divp(nullptr)
		, d_(0)
		, a_(0)
	{
	}
	bool init(uint32_t d, int /*mode*/ = 0)
	{
		using namespace Xbyak;
		using namespace Xbyak::util;
		{
			ConstDiv cd;
			if (!cd.init(d)) return false;
			d_ = cd.d_;
			a_ = cd.a_;
			StackFrame sf(this, 1, UseRDX);
			const Reg32 x = sf.p[0].cvt32();
			if (d >= 0x80000000) {
				xor_(eax, eax);
				cmp(x, d);
				setae(al);
			} else {
				if (cd.c_ <= 0xffffffff) {
					mov(eax, x);
					if (cd.c_ > 1) {
						mov(edx, cd.c_);
						mul(rdx);
					}
					shr(rax, cd.a_);
				} else {
					mov(eax, x);
					mov(rdx, cd.c_);
					mul(rdx);
					shrd(rax, rdx, cd.a_);
				}
			}
		}
		setProtectModeRE();
		divp = getCode<DivFunc>();
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
		printf("Gen d=%u(0x%08x) a=%u divp=%p\n", d_, d_, a_, divp);
	}
};

