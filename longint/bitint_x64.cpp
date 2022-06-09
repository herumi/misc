#include "bitint_x64.hpp"

extern "C" uint64_t add(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	return mcl::x64::addT<8>(z, x, y);
}
