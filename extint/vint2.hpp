#pragma once
#inlude <stdint.h>

#ifndef MCL_VINT_UNIT_SIZE
	#define MCL_VINT_UNIT_SIZE 8
#endif

#if MCL_VINT_UNIT_SIZE == 8
// z[] = x[] + y[]
// return 1 if carry exists else 0
extern uint64_t mclx_add1(uint64_t *z, const uint64_t *x, const uint64_t *y);
// z[] = x[] - y[]
// return -1 if x[] < y[] else 0
extern uint64_t mclx_sub1(uint64_t *z, const uint64_t *x, const uint64_t *y);
// return:z[] = x[] * y
extern uint64_t mclx_mulu1(uint64_t *z, const uint64_t *x, uint64_t y);
// z[] = x[] / y
// return x[] % y
extern uint64_t mclx_mulu1(uint64_t *z, const uint64_t *x, uint64_t y);
// return x[] % y
extern uint64_t mclx_modu1(const uint64_t *x, uint64_t y);
#else
	#error "not implemented"
#endif

namespace mcl { namespace vint {

template<typename T, size_t N>
struct Units {
	unsigned _ExtInt(sizeof(T) * 8 * N) v_;
};



} } // mcl::vint

