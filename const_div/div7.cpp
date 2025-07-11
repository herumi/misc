#include "uint128_t.hpp"

const uint64_t u_ = 0x124924925;
const uint32_t a_=35;

extern "C" {

uint32_t div7org(uint32_t x)
{
	return x / 7;
}

uint32_t div19org(uint32_t x)
{
	return x / 19;
}

uint32_t div7org2(uint32_t x)
{
	uint64_t v = x * (u_ & 0xffffffff);
	v >>= 32;
	uint32_t xL = uint32_t(x) - uint32_t(v);
	xL >>= 1;
	xL += uint32_t(v);
	xL >>= 2;
	return xL;
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
