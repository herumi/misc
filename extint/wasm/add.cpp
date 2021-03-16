#include "api.h"
#include "../exti.hpp"

inline exti256_t* cast256(uint32_t *x) { return (exti256_t*)x; }
inline const exti256_t* cast256(const uint32_t *x) { return (const exti256_t*)x; }
inline exti512_t* cast512(uint32_t *x) { return (exti512_t*)x; }
inline const exti512_t* cast512(const uint32_t *x) { return (const exti512_t*)x; }

template<uint32_t N>
void addT64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	uint32_t c = 0;
	for (uint32_t i = 0; i < N; i++) {
		uint64_t xc = x[i] + c;
		if (xc < c) {
			// x[i] = Unit(-1) and c = 1
			z[i] = y[i];
		} else {
			xc += y[i];
			c = y[i] > xc ? 1 : 0;
			z[i] = xc;
		}
	}
}

template<uint32_t N>
uint32_t addT32(uint32_t z[N], const uint32_t x[N], const uint32_t y[N])
{
	uint32_t c = 0;
	for (uint32_t i = 0; i < N; i++) {
		uint64_t v = uint64_t(x[i]) + y[i] + c;
		z[i] = uint32_t(v);
		c = uint32_t(v >> 32);
	}
	return c;
}


extern "C" {

API void add256_extInt(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	*cast256(z) = *cast256(x) + *cast256(y);
}

API void add256_u32(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	addT32<8>(z, x, y);
}

API void add256_u64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	addT64<4>(z, x, y);
}

}

