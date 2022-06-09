#include "bitint_x64.hpp"

#if 1
extern "C" uint64_t xxx(uint64_t *z, const uint64_t *x, uint64_t y)
{
	return mcl::x64::mulUnitT<1>(z, x, y);
}
extern "C" uint64_t xxx2(uint64_t *z, const uint64_t *x, uint64_t y)
{
	return mcl::x64::mulUnitT<2>(z, x, y);
}
extern "C" uint64_t xxx4(uint64_t *z, const uint64_t *x, uint64_t y)
{
	return mcl::x64::mulUnitT<4>(z, x, y);
}
#else
extern "C" uint64_t xxx(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	return mcl::x64::subT<8>(z, x, y);
}
#endif

