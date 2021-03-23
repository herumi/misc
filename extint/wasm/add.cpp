#include "api.h"
#include "../exti.hpp"

#define CYBOZU_ALLOCA(x) __builtin_alloca(x)
#define MCL_SIZEOF_UNIT 8
#define MCL_VINT_64BIT_PORTABLE

typedef uint32_t size_t;

inline exti256_t* cast256(uint32_t *x) { return (exti256_t*)x; }
inline const exti256_t* cast256(const uint32_t *x) { return (const exti256_t*)x; }
inline exti512_t* cast512(uint32_t *x) { return (exti512_t*)x; }
inline const exti512_t* cast512(const uint32_t *x) { return (const exti512_t*)x; }

template<uint32_t N>
void addT64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	uint32_t c = 0;
	for (uint32_t i = 0; i < N; i++) {
		uint64_t xc = x[i] + c;
		if (xc < c) {
			// x[i] = Unit(-1) and c = 1
			z[i] = y[i];
		} else {
			xc += y[i];
			c = y[i] > xc ? 1 : 0;
			z[i] = xc;
		}
	}
}

template<uint32_t N>
uint32_t addT32(uint32_t z[N], const uint32_t x[N], const uint32_t y[N])
{
	uint32_t c = 0;
	for (uint32_t i = 0; i < N; i++) {
		uint64_t v = uint64_t(x[i]) + y[i] + c;
		z[i] = uint32_t(v);
		c = uint32_t(v >> 32);
	}
	return c;
}

namespace vint {

namespace fp {
template<class T>
void swap_(T& x, T& y)
{
	T t;
	t = x;
	x = y;
	y = t;
}

} // fp

inline void split64(uint32_t *H, uint32_t *L, uint64_t x)
{
	*H = uint32_t(x >> 32);
	*L = uint32_t(x);
}

inline uint32_t mulUnit(uint32_t *pH, uint32_t x, uint32_t y)
{
	uint64_t t = uint64_t(x) * y;
	uint32_t L;
	split64(pH, &L, t);
	return L;
}
#if MCL_SIZEOF_UNIT == 8
inline uint64_t mulUnit(uint64_t *pH, uint64_t x, uint64_t y)
{
#ifdef MCL_VINT_64BIT_PORTABLE
	const uint64_t mask = 0xffffffff;
	uint64_t v = (x & mask) * (y & mask);
	uint64_t L = uint32_t(v);
	uint64_t H = v >> 32;
	uint64_t ad = (x & mask) * uint32_t(y >> 32);
	uint64_t bc = uint32_t(x >> 32) * (y & mask);
	H += uint32_t(ad);
	H += uint32_t(bc);
	L |= H << 32;
	H >>= 32;
	H += ad >> 32;
	H += bc >> 32;
	H += (x >> 32) * (y >> 32);
	*pH = H;
	return L;
#elif defined(_WIN64) && !defined(__INTEL_COMPILER)
	return _umul128(x, y, pH);
#else
	typedef __attribute__((mode(TI))) unsigned int uint128;
	uint128 t = uint128(x) * y;
	*pH = uint64_t(t >> 64);
	return uint64_t(t);
#endif
}
#endif

template<class T>
T addN(T *z, const T *x, const T *y, size_t n)
{
	T c = 0;
	for (size_t i = 0; i < n; i++) {
		T xc = x[i] + c;
		if (xc < c) {
			// x[i] = Unit(-1) and c = 1
			z[i] = y[i];
		} else {
			xc += y[i];
			c = y[i] > xc ? 1 : 0;
			z[i] = xc;
		}
	}
	return c;
}


template<class T>
T mulu1(T *z, const T *x, size_t n, T y)
{
	T H = 0;
	for (size_t i = 0; i < n; i++) {
		T t = H;
		T L = mulUnit(&H, x[i], y);
		z[i] = t + L;
		if (z[i] < t) {
			H++;
		}
	}
	return H; // z[n]
}

template<class T>
void copyN(T *y, const T *x, size_t n)
{
	for (size_t i = 0; i < n; i++) y[i] = x[i];
}
template<class T>
void clearN(T *x, size_t n)
{
	for (size_t i = 0; i < n; i++) x[i] = 0;
}

/*
	z[xn * yn] = x[xn] * y[ym]
*/
template<class T>
static inline void mulNM(T *z, const T *x, size_t xn, const T *y, size_t yn)
{
	if (yn > xn) {
		fp::swap_(yn, xn);
		fp::swap_(x, y);
	}
	if (z == x) {
		T *p = (T*)CYBOZU_ALLOCA(sizeof(T) * xn);
		copyN(p, x, xn);
		x = p;
	}
	if (z == y) {
		T *p = (T*)CYBOZU_ALLOCA(sizeof(T) * yn);
		copyN(p, y, yn);
		y = p;
	}
	z[xn] = vint::mulu1(&z[0], x, xn, y[0]);
	clearN(z + xn + 1, yn - 1);

	T *t2 = (T*)CYBOZU_ALLOCA(sizeof(T) * (xn + 1));
	for (size_t i = 1; i < yn; i++) {
		t2[xn] = vint::mulu1(&t2[0], x, xn, y[i]);
		vint::addN(&z[i], &z[i], &t2[0], xn + 1);
	}
}

} // vint

namespace u32 {

#include "src/low_func_wasm.hpp"

} // u32


extern "C" {

API void add256_extInt(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	*cast256(z) = *cast256(x) + *cast256(y);
}

#if 0

API void mulPre256_extInt(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	exti256_t tx = (exti256_t)*(const unsigned _ExtInt(128)*)x;
	exti256_t ty = (exti256_t)*(const unsigned _ExtInt(128)*)y;
	*cast256(z) = tx * ty;
}
#endif

API void add256_u32(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	addT32<8>(z, x, y);
}

API void mul256_u32(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	u32::mcl::mulT<8>(z, x, y);
}

API void add256_u64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	addT64<4>(z, x, y);
}

API void mul256_u64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	vint::mulNM<uint64_t>(z, x, 4, y, 4);
}

API void add384_u32(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	addT32<12>(z, x, y);
}

API void mul384_u32(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	u32::mcl::mulT<12>(z, x, y);
}

API void add384_u64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	addT64<6>(z, x, y);
}

API void mul384_u64(uint64_t *z, const uint64_t *x, const uint64_t *y)
{
	vint::mulNM<uint64_t>(z, x, 6, y, 6);
}


}

