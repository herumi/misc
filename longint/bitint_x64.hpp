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

#if 0
// z[N] = x[N] - y[N] and return CF(0 or 1)
template<size_t N>uint64_t subT(uint64_t *z, const uint64_t *x, const uint64_t *y);
// [ret:z[N]] = x[N] * y
template<size_t N>uint64_t mulUnitT(uint64_t *z, const uint64_t *x, uint64_t y);
// [ret:z[N]] = z[N] + x[N] * y
template<size_t N>uint64_t mulUnitAddT(uint64_t *z, const uint64_t *x, uint64_t y);
#endif

} } // mcl::x64

