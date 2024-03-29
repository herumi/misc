#include "bitint.hpp"
#include "bitint_if.hpp"

extern "C" {

size_t mclb_divFullBit256(uint64_t *q, size_t qn, uint64_t *x, size_t xn, const uint64_t *y)
{
#if MCL_SIZEOF_UNIT == 8
	return mcl::bint::divFullBitT<4>(q, qn, x, xn, y);
#endif
}

} // extern "C"
