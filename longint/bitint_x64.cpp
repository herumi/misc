#include "bitint_x64.hpp"

#if 1
extern "C" uint64_t xxx(uint64_t *z, const uint64_t *x, uint64_t y)
{
	return mcl::x64::mulUnitAddT<4>(z, x, y);
}
#else
extern "C" uint64_t xxx(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	return mcl::x64::subT<8>(z, x, y);
}
#endif

