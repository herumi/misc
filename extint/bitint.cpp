#include "bitint.hpp"

extern "C" size_t divFullBit4(uint64_t *q, size_t qn, uint64_t *x, size_t xn, const uint64_t *y)
{
	return mcl::vint::divFullBitT<4>(q, qn, x, xn, y);
}
