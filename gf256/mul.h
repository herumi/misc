#pragma once

#include <stdint.h>
#include <stdio.h>

typedef bool F2;
typedef uint8_t K;
typedef uint16_t K2;

inline K mulX(K a)
{
	if (a & 0x80) {
		a = (a << 1) ^ 0x1b;
	}else{
		a <<= 1;
	}
	return a;
}
inline K mul(K a, K b)
{
	K ret = 0;
	while (b) {
		if (b & 1) ret ^= a;
		a = mulX(a);
		b >>= 1;
	}
	return ret;
}

template<class T>
F2 get(T a, int i) { return (a >> i) & 1; }
template<class T>
void set(T& a, int i, F2 b)
{
	a &= ~(1ull<<i);
	if (b) a |= 1ull<<i;
}
template<class T>
void put(T a, int n = 8)
{
	for (int i = 0; i < n; i++) {
		putchar('0' + ((a>>(n-1-i))&1));
	}
	puts("");
}
K2 mulPoly(K a, K b)
{
	const int n = 8;
	K2 ret = 0;
	for (int i = 0; i < n; i++) {
		F2 r = 0;
		for (int j = 0; j <= i; j++) r ^= get(a, j) & get(b, i-j);
		set(ret, i, r);
	}
	for (int i = n; i < 2*n-1; i++) {
		F2 r = 0;
		for (int j = i-n+1; j < n; j++) r ^= get(a, j) & get(b, i-j);
		set(ret, i, r);
	}
	return ret;
}

K modPoly(K2 c)
{
	const int n = 8;
	for (int i = 2*n-2; i >= n; i--) {
		if (get(c, i)) {
			c ^= 0x1b << (i-n);
		}
	}
	return c;
}

K mulMod(K a, K b)
{
	return modPoly(mulPoly(a, b));
}
