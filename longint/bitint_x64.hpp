#pragma once
/*
	@file
	@brief low level functions
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/

#include <assert.h>
#include <stdint.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#if (__cplusplus < 201703L) && (_MSC_VER < 1920) // VS2019
	#error "require -std=c++17 for clang or /std:c++17 for Visual Studio 2019"
#endif

namespace mcl { namespace x64 {

// z[N] = x[N] + y[N] and return CF(0 or 1)
template<size_t N, size_t I = 0>
uint64_t addT(uint64_t *z, const uint64_t *x, const uint64_t *y, uint8_t c = 0)
{
	if constexpr (I < N) {
		c = _addcarry_u64(c, x[I], y[I], (unsigned long long *)&z[I]);
		return addT<N, I + 1>(z, x, y, c);
	} else {
		return c;
	}
}

// z[N] = x[N] - y[N] and return CF(0 or 1)
template<size_t N, size_t I = 0>
uint64_t subT(uint64_t *z, const uint64_t *x, const uint64_t *y, uint8_t c = 0)
{
	if constexpr (I < N) {
		c = _subborrow_u64(c, x[I], y[I], (unsigned long long *)&z[I]);
		return subT<N, I + 1>(z, x, y, c);
	} else {
		return c;
	}
}

// [ret:z[N]] = x[N] * y
template<size_t N, size_t I = 0>
uint64_t mulUnitT(uint64_t *z, const uint64_t *x, uint64_t y, uint8_t c = 0, unsigned long long L = 0)
{
	if constexpr (I == 0) {
		z[0] = _mulx_u64(x[0], y, &L);
		return mulUnitT<N, I + 1>(z, x, y, c, L);
	}
	if constexpr (I < N) {
		unsigned long long H;
		uint64_t t = _mulx_u64(x[I], y, &H);
		c = _addcarry_u64(c, t, L, (unsigned long long *)&z[I]);
		return mulUnitT<N, I + 1>(z, x, y, c, H);
	}
	unsigned long long H;
	_addcarry_u64(c, 0, L, &H);
	return H;
}


#if 0
// [ret:z[N]] = z[N] + x[N] * y
template<size_t N>uint64_t mulUnitAddT(uint64_t *z, const uint64_t *x, uint64_t y);
#endif

} } // mcl::x64

