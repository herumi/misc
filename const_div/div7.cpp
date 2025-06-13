#include <inttypes.h>

#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
typedef __attribute__((mode(TI))) unsigned int uint128_t;
#define MCL_DEFINED_UINT128_T
#endif

inline uint64_t mulUnit1(uint64_t *pH, uint64_t x, uint64_t y)
{
#ifdef MCL_DEFINED_UINT128_T
	uint128_t t = uint128_t(x) * y;
	*pH = uint64_t(t >> 64);
	return uint64_t(t);
#else
	return _umul128(x, y, pH);
#endif
}

const uint64_t u_ = 0x124924925;
const uint32_t a_=35;

extern "C" {

uint32_t div7org(uint32_t x)
{
	return x / 7;
}

uint32_t div7a(uint32_t x)
{
#ifdef MCL_DEFINED_UINT128_T
//	__builtin_assume(a_ >= 32);
	uint128_t v = (x * uint128_t(u_)) >> a_;
	return uint32_t(v);
#else
	uint64_t H;
	uint64_t L = mulUnit1(&H, x, u_);
	L >>= a_;
	H <<= (64 - a_);
	return uint32_t(H | L);
#endif
}

uint32_t div7b(uint32_t x)
{
	uint64_t v = x * (u_ & 0xffffffff);
	v >>= 32;
	v += x;
	v >>= a_-32;
	return uint32_t(v);
}

} // extern "C"
