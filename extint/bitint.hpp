#pragma once

#include <mcl/config.hpp>
#include <assert.h>

extern "C" size_t divFullBit4(uint64_t *q, size_t qn, uint64_t *x, size_t xn, const uint64_t *y);

namespace mcl { namespace vint {

inline uint64_t make64(uint32_t H, uint32_t L)
{
	return ((uint64_t)H << 32) | L;
}

inline void split64(uint32_t *H, uint32_t *L, uint64_t x)
{
	*H = uint32_t(x >> 32);
	*L = uint32_t(x);
}

// return the real size of x
// return 1 if x[n] == 0
inline size_t getRealSize(const fp::Unit *x, size_t n)
{
	while (n > 0) {
		if (x[n - 1]) break;
		n--;
	}
	return n > 0 ? n : 1;
}

/*
	[H:L] <= x * y
	@return L
*/
inline uint32_t mulUnit(uint32_t *pH, uint32_t x, uint32_t y)
{
	uint64_t t = uint64_t(x) * y;
	uint32_t L;
	split64(pH, &L, t);
	return L;
}

/*
	q = [H:L] / y
	r = [H:L] % y
	return q
*/
inline uint32_t divUnit(uint32_t *pr, uint32_t H, uint32_t L, uint32_t y)
{
	assert(y != 0);
	uint64_t t = make64(H, L);
	uint32_t q = uint32_t(t / y);
	*pr = uint32_t(t % y);
	return q;
}

#if MCL_SIZEOF_UNIT == 8
inline uint64_t mulUnit(uint64_t *pH, uint64_t x, uint64_t y)
{
#if defined(_WIN64) && !defined(__INTEL_COMPILER)
	return _umul128(x, y, pH);
#else
	typedef __attribute__((mode(TI))) unsigned int uint128;
	uint128 t = uint128(x) * y;
	*pH = uint64_t(t >> 64);
	return uint64_t(t);
#endif
}

inline uint64_t divUnit(uint64_t *pr, uint64_t H, uint64_t L, uint64_t y)
{
	assert(y != 0);
#ifdef _MSC_VER
	return _udiv128(H, L, y, pr);
#else
	typedef __attribute__((mode(TI))) unsigned int uint128;
	uint128 t = (uint128(H) << 64) | L;
	uint64_t q = uint64_t(t / y);
	*pr = uint64_t(t % y);
	return q;
#endif
}

#endif // MCL_SIZEOF_UNIT == 8

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

// z[n] = x[n] + y
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
// x[n] += y
inline fp::Unit addUnit(fp::Unit *x, size_t n, fp::Unit y)
{
	assert(n > 0);
	fp::Unit t = x[0] + y;
	x[0] = t;
	size_t i = 0;
	if (t >= y) return 0;
	i = 1;
	for (; i < n; i++) {
		t = x[i] + 1;
		x[i] = t;
		if (t != 0) return 0;
	}
	return 1;
}

// z[n] = x[n] - y
inline fp::Unit subUnit(fp::Unit *z, const fp::Unit *x, size_t n, fp::Unit y)
{
	assert(n > 0);
	fp::Unit c = x[0] < y ? 1 : 0;
	z[0] = x[0] - y;
	for (size_t i = 1; i < n; i++) {
		if (x[i] < c) {
			z[i] = fp::Unit(-1);
		} else {
			z[i] = x[i] - c;
			c = 0;
		}
	}
	return c;
}

/*
	q[] = x[] / y
	@retval r = x[] % y
	accept q == x
*/
inline fp::Unit divUnit(fp::Unit *q, const fp::Unit *x, size_t n, fp::Unit y)
{
	assert(n > 0);
	fp::Unit r = 0;
	for (int i = (int)n - 1; i >= 0; i--) {
		q[i] = divUnit(&r, r, x[i], y);
	}
	return r;
}
/*
	q[] = x[] / y
	@retval r = x[] % y
*/
inline fp::Unit modUnit(const fp::Unit *x, size_t n, fp::Unit y)
{
	assert(n > 0);
	fp::Unit r = 0;
	for (int i = (int)n - 1; i >= 0; i--) {
		divUnit(&r, r, x[i], y);
	}
	return r;
}

// x[n] = 0
inline void clear(fp::Unit *x, size_t n)
{
	for (size_t i = 0; i < n; i++) x[i] = 0;
}

/*
	y must be sizeof(Unit) * 8 * N bit
	x[xn] = x[xn] % y[N]
	q[qn] = x[xn] / y[N] if q != NULL
	return new xn
*/
template<size_t N>
size_t divFullBitT(fp::Unit *q, size_t qn, fp::Unit *x, size_t xn, const fp::Unit *y)
{
	assert(xn > 0);
	assert((y[N - 1] >> (sizeof(fp::Unit) * 8 - 1)) != 0);
	if (q) clear(q, qn);
	fp::Unit *t = (fp::Unit*)CYBOZU_ALLOCA(sizeof(fp::Unit) * (N + 1));
	while (xn > N) {
		if (x[xn - 1] == 0) {
			xn--;
			continue;
		}
		size_t d = xn - N;
		if (cmpGeT<N>(x + d, y)) {
			subT<N>(x + d, x + d, y);
			if (q) addUnit(q + d, qn - d, 1);
		} else {
			fp::Unit xTop = x[xn - 1];
			if (xTop == 1) {
				fp::Unit ret= subT<N>(x + d - 1, x + d - 1, y);
				x[xn-1] -= ret;
			} else {
				t[N] = mulUnitT<N>(t, y, xTop);
				subT<N + 1>(x + d - 1, x + d - 1, t);
			}
			if (q) addUnit(q + d - 1, qn - d + 1, xTop);
		}
	}
	if (cmpGeT<N>(x, y)) {
		subT<N>(x, x, y);
		if (q) addUnit(q, qn, 1);
	}
	xn = getRealSize(x, xn);
	return xn;
}

} } // mcl::vint

