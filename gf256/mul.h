#pragma once

#include <stdint.h>
#include <stdio.h>

typedef bool F2;
typedef uint8_t K;
typedef uint16_t K2;

inline K mul(K x, K y)
{
	K ret = 0;
	while (x && y) {
		if (y & 1) ret ^= x;
		if (x & 0x80) {
			x = (x << 1) ^ 0x11b;
		}else{
			x <<= 1;
		}
		y >>= 1;
	}
	return ret;
}

F2 get(K a, int i) { return (a >> i) & 1; }
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
	for (int i = 2*n-2; i >= 8; i--) {
		if (get(c, i)) {
			c ^= 0x11b << (i-8);
		}
	}
	return c;
}
