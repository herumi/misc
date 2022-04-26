#pragma once
/*
	interface of bitint
*/
#include <mcl/config.hpp>

namespace mcl { namespace bint {

size_t divFullBit256(fp::Unit *q, size_t qn, fp::Unit *x, size_t xn, const fp::Unit *y);

} } // mcl::bint
