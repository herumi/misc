#pragma once
/*
	interface of bitint
*/
#include <mcl/config.hpp>


extern "C" {

size_t mclb_divFullBit256(mcl::fp::Unit *q, size_t qn, mcl::fp::Unit *x, size_t xn, const mcl::fp::Unit *y);

} // extern "C"
