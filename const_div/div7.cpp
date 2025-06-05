
#include <stdint.h>

typedef __attribute__((mode(TI))) unsigned int uint128_t;
const uint64_t u_ = 0x124924925;
const uint32_t a_=35;

extern "C" {

uint32_t div7org(uint32_t x)
{
	return x / 7;
}

uint32_t div7a(uint32_t x)
{
    uint128_t v = (x * uint128_t(u_)) >> a_;
    return uint32_t(v);
}

uint32_t div7b(uint32_t x)
{
	uint64_t v = x * (u_ & 0xffffffff);
	v >>= 32;
	v += x;
	v >>= a_-32;
	return uint32_t(v);
}

}
