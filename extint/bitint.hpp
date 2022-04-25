#pragma once

#include <mcl/config.hpp>
#include <assert.h>

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

// true if x[N] == y[N]
template<size_t N>
bool cmpEqT(const fp::Unit *px, const fp::Unit *py)
{
	const auto& x = BitInt<N>::load(px);
	const auto& y = BitInt<N>::load(py);
	return x.v == y.v;
}

// true if x[N] >= y[N]
template<size_t N>
bool cmpGeT(const fp::Unit *px, const fp::Unit *py)
{
	const auto& x = BitInt<N>::load(px);
	const auto& y = BitInt<N>::load(py);
	return x.v >= y.v;
}

// true if x[N] > y[N]
template<size_t N>
bool cmpGtT(const fp::Unit *px, const fp::Unit *py)
{
	const auto& x = BitInt<N>::load(px);
	const auto& y = BitInt<N>::load(py);
	return x.v > y.v;
}

// true if x[N] <= y[N]
template<size_t N>
bool cmpLeT(const fp::Unit *px, const fp::Unit *py)
{
	return !cmpGtT<N>(px, py);
}

// true if x[N] < y[N]
template<size_t N>
bool cmpLtT(const fp::Unit *px, const fp::Unit *py)
{
	return !cmpGeT<N>(px, py);
}

// z[N] = x[N] + y[N] and return CF(0 or 1)
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

// z[N] = x[N] - y[N] and return CF(0 or 1)
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

// [ret:z[N]] = x[N] * y
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

// [ret:z[N]] = z[N] + x[N] * y
template<size_t N>
fp::Unit mulUnitAddT(fp::Unit *pz, const fp::Unit *px, fp::Unit y)
{
	auto x = BitInt<N>::load(px).template cvt<N+1>();
	auto z = BitInt<N>::load(pz).template cvt<N+1>();
	BitInt<1> y1;
	y1.v = y;
	z.v += x.v * y1.template cvt<N+1>().v;
	z.template cvt<N>().save(pz);
	return z.getTopUnit();
}

// z[2N] = x[N] * y[N]
template<size_t N>
void mulT(fp::Unit *pz, const fp::Unit *px, const fp::Unit *py)
{
	pz[N] = mulUnitT<N>(pz, px, py[0]);
	for (size_t i = 1; i < N; i++) {
		pz[N + i] = mulUnitAddT<N>(&pz[i], px, py[i]);
	}
}

// [ret:z[N]] = x[N] << y
template<size_t N>
fp::Unit shlT(fp::Unit *pz, const fp::Unit *px, fp::Unit y)
{
	assert(0 < y && y < sizeof(fp::Unit) * 8);
	auto x = BitInt<N>::load(px).template cvt<N+1>();
	BitInt<N+1> z;
	z.v = x.v << y;
	z.template cvt<N>().save(pz);
	return z.getTopUnit();
}

// z[N] = x[N] >> y
template<size_t N>
void shrT(fp::Unit *pz, const fp::Unit *px, fp::Unit y)
{
	assert(0 < y && y < sizeof(fp::Unit) * 8);
	auto x = BitInt<N>::load(px);
	x.v >>= y;
	x.template cvt<N>().save(pz);
}

/*
	z[n] = x[n] + y
*/
inline fp::Unit addUnit(fp::Unit *z, const fp::Unit *x, size_t n, fp::Unit y)
{
	assert(n > 0);
	fp::Unit t = x[0] + y;
	z[0] = t;
	size_t i = 0;
	if (t >= y) goto EXIT_0;
	i = 1;
	for (; i < n; i++) {
		t = x[i] + 1;
		z[i] = t;
		if (t != 0) goto EXIT_0;
	}
	return 1;
EXIT_0:
	i++;
	for (; i < n; i++) {
		z[i] = x[i];
	}
	return 0;
}


} } // mcl::vint

