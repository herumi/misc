#pragma once

#include <mcl/config.hpp>

namespace mcl { namespace vint {

template<size_t N>
struct BitInt {
	unsigned _ExtInt(sizeof(fp::Unit) * 8 * N) v;
};

// true if x >= y
template<size_t N>
bool cmpGeT(const fp::Unit *px, const fp::Unit *py)
{
	const BitInt<N>& x = *(const BitInt<N>*)px;
	const BitInt<N>& y = *(const BitInt<N>*)py;
	return x.v >= y.v;
}

// true if x > y
template<size_t N>
bool cmpGtT(const fp::Unit *px, const fp::Unit *py)
{
	const BitInt<N>& x = *(const BitInt<N>*)px;
	const BitInt<N>& y = *(const BitInt<N>*)py;
	return x.v > y.v;
}

// true if x <= y
template<size_t N>
bool cmpLeT(const fp::Unit *px, const fp::Unit *py)
{
	return cmpGtT<N>(py, px);
}

// true if x < y
template<size_t N>
bool cmpLtT(const fp::Unit *px, const fp::Unit *py)
{
	return cmpGeT<N>(py, px);
}

} } // mcl::vint

