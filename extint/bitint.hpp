#pragma once

#include <mcl/config.hpp>

namespace mcl { namespace vint {

template<size_t N>
struct BitInt {
	static const size_t bitSize = sizeof(fp::Unit) * 8 * N;
	typedef unsigned _ExtInt(bitSize) Type;
	Type v;
	fp::Unit getTopUnit() const
	{
		return static_cast<fp::Unit>(v >> (bitSize - sizeof(fp::Unit) * 8));
	}
	fp::Unit getMSB() const
	{
		return getTopUnit() >> (sizeof(fp::Unit) * 8 - 1);
	}
	static const BitInt<N>& load(const void *x)
	{
		return *(const BitInt<N>*)x;
	}
	void save(void *x) const
	{
		*(BitInt<N>*)(x) = *this;
	}
	template<size_t M>
	BitInt<M> cvt() const
	{
		BitInt<M> ret;
		ret.v = static_cast<typename BitInt<M>::Type>(this->v);
		return ret;
	}
};

// true if x == y
template<size_t N>
bool cmpEqT(const fp::Unit *px, const fp::Unit *py)
{
	const auto& x = BitInt<N>::load(px);
	const auto& y = BitInt<N>::load(py);
	return x.v == y.v;
}

// true if x >= y
template<size_t N>
bool cmpGeT(const fp::Unit *px, const fp::Unit *py)
{
	const auto& x = BitInt<N>::load(px);
	const auto& y = BitInt<N>::load(py);
	return x.v >= y.v;
}

// true if x > y
template<size_t N>
bool cmpGtT(const fp::Unit *px, const fp::Unit *py)
{
	const auto& x = BitInt<N>::load(px);
	const auto& y = BitInt<N>::load(py);
	return x.v > y.v;
}

// true if x <= y
template<size_t N>
bool cmpLeT(const fp::Unit *px, const fp::Unit *py)
{
	return !cmpGtT<N>(px, py);
}

// true if x < y
template<size_t N>
bool cmpLtT(const fp::Unit *px, const fp::Unit *py)
{
	return !cmpGeT<N>(px, py);
}

// z = x + y and return CF(0 or 1)
template<size_t N>
fp::Unit addT(fp::Unit *pz, const fp::Unit *px, const fp::Unit *py)
{
	auto x = BitInt<N>::load(px).template cvt<N+1>();
	auto y = BitInt<N>::load(py).template cvt<N+1>();
	BitInt<N+1> z;
	z.v = x.v + y.v;
	z.template cvt<N>().save(pz);
	return z.getTopUnit();
}

// z = x - y and return CF(0 or 1)
template<size_t N>
fp::Unit subT(fp::Unit *pz, const fp::Unit *px, const fp::Unit *py)
{
	auto x = BitInt<N>::load(px).template cvt<N+1>();
	auto y = BitInt<N>::load(py).template cvt<N+1>();
	BitInt<N+1> z;
	z.v = x.v - y.v;
	z.template cvt<N>().save(pz);
	return z.getMSB();
}

// [ret:z] = x * y
template<size_t N>
fp::Unit mulUnitT(fp::Unit *pz, const fp::Unit *px, fp::Unit y)
{
	auto x = BitInt<N>::load(px).template cvt<N+1>();
	BitInt<1> y1;
	BitInt<N+1> z;
	y1.v = y;
	z.v = x.v * y1.template cvt<N+1>().v;
	z.template cvt<N>().save(pz);
	return z.getTopUnit();
}

// [ret:w] = x + y * z
template<size_t N>
fp::Unit mulUnitAddT(fp::Unit *pw, const fp::Unit *px, const fp::Unit *py, fp::Unit z)
{
	auto x = BitInt<N>::load(px).template cvt<N+1>();
	auto y = BitInt<N>::load(py).template cvt<N+1>();
	BitInt<1> z1;
	BitInt<N+1> w;
	z1.v = z;
	w.v = x.v + y.v * z1.template cvt<N+1>().v;
	w.template cvt<N>().save(pw);
	return w.getTopUnit();
}

} } // mcl::vint

