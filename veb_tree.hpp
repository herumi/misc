#pragma once
/*
	original is http://code.google.com/p/libveb/
	MIT License
*/
#include <cybozu/bit_operation.hpp>
#include <assert.h>

namespace veb_tree_local {

const uint32_t bitSize = (uint32_t) sizeof(uint32_t) * 8;

inline uint32_t fls(uint32_t x)
{
	assert(x);
	return 1 + cybozu::bsr(x);
}

struct VebSub {
	uint32_t M;
	uint32_t k;
	uint8_t *p;
};

inline uint32_t bytes(uint32_t x)
{
	return x / 8 + ((x % 8) > 0);
}

inline uint32_t zeros(uint32_t k)
{
	return (~0) << k;
}

inline uint32_t ones(uint32_t k)
{
	return ~zeros(k);
}

inline uint32_t ipow(uint32_t k)
{
	return 1 << k;
}

inline uint32_t lowbits(uint32_t x, uint32_t k)
{
	return x & ones(k);
}

inline uint32_t highbits(uint32_t x, uint32_t k)
{
	return x >> k;
}

inline uint32_t decode(const uint8_t p[], uint32_t b)
{
	uint32_t x = 0;
	for (uint32_t i = 0; i < b; i++) {
		x |= p[i] << 8 * i;
	}
	return x;
}

inline void encode(uint8_t p[], uint32_t b, uint32_t x)
{
	for (uint32_t i = 0; i < b; ++i) {
		p[i] = (x >> 8 * i) & 0xff;
	}
}

inline void set(uint8_t p[], uint32_t x)
{
	p[x / 8] |= 1 << (x % 8);
}

inline void unset(uint8_t p[], uint32_t x)
{
	p[x / 8] &= ~(1 << (x % 8));
}

inline uint32_t low(const VebSub& T)
{
	if (T.M <= bitSize) {
		uint32_t x = decode(T.p, bytes(T.M));
		if (x == 0) return T.M;
		return cybozu::bsf(x);
	}
	return decode(T.p, bytes(T.k));
}

inline void setlow(VebSub& T, uint32_t x)
{
	if (T.M <= bitSize) {
		set(T.p, x);
	} else {
		encode(T.p, bytes(T.k), x);
	}
}

inline uint32_t high(const VebSub& T)
{
	if (T.M <= bitSize) {
		uint32_t x = decode(T.p, bytes(T.M));
		if (x == 0) return T.M;
		return cybozu::bsr(x);
	}
	return decode(T.p + bytes(T.k), bytes(T.k));
}

inline void sethigh(VebSub& T, uint32_t x)
{
	if (T.M <= bitSize) {
		set(T.p, x);
	} else {
		encode(T.p + bytes(T.k), bytes(T.k), x);
	}
}

inline uint32_t vebsize(uint32_t M)
{
	if (M <= bitSize) return bytes(M);
	uint32_t k = fls(M - 1);
	uint32_t m = highbits(M - 1, k / 2) + 1;
	uint32_t n = ipow(k / 2);
	return 2 * bytes(k) + vebsize(m) + (m - 1) * vebsize(n) + vebsize(M - (m - 1) * n);
}

inline void aux(VebSub& T, const VebSub& S)
{
	T.k = S.k - S.k / 2;
	T.p = S.p + 2 * bytes(S.k);
	T.M = highbits(S.M - 1, S.k / 2) + 1;
}

inline void branch(VebSub& T, const VebSub& S, uint32_t i)
{
	uint32_t k = S.k / 2;
	uint32_t m = highbits(S.M - 1, k) + 1;
	uint32_t n = ipow(k);
	if (i < m - 1) {
		T.M = n;
		T.k = k;
	} else {
		T.M = S.M - (m - 1) * n;
		T.k = fls(T.M - 1);
	}
	T.p = S.p + 2 * bytes(S.k) + vebsize(m) + i * vebsize(n);
}

inline bool empty(const VebSub& T)
{
	if (T.M <= bitSize) return decode(T.p, bytes(T.M)) == 0;
	if (low(T) <= high(T)) return false;
	return true;
}

inline void mkempty(VebSub& T)
{
	if (T.M <= bitSize) {
		encode(T.p, bytes(T.M), 0);
		return;
	}
	setlow(T, 1);
	sethigh(T, 0);
	VebSub A;
	aux(A, T);
	mkempty(A);
	uint32_t m = highbits(T.M - 1, T.k / 2) + 1;
	for (uint32_t i = 0; i < m; i++) {
		VebSub B;
		branch(B, T, i);
		mkempty(B);
	}
}

inline void vebInsert(VebSub& T, uint32_t x)
{
	if (x >= T.M) return;
	if (T.M <= bitSize) {
		set(T.p, x);
		return;
	}
	if (empty(T)) {
		setlow(T, x);
		sethigh(T, x);
		return;
	}
	uint32_t lo = low(T);
	uint32_t hi = high(T);
	if (x == lo || x == hi) return;
	if (x < lo) {
		setlow(T, x);
		if (lo == hi) return;
		x = lo;
	} else if (x > hi) {
		sethigh(T, x);
		if (lo == hi) return;
		x = hi;
	}
	uint32_t i = highbits(x, T.k / 2);
	uint32_t j = lowbits(x, T.k / 2);
	VebSub B;
	branch(B, T, i);
	vebInsert(B, j);
	if (low(B) == high(B)) {
		VebSub A;
		aux(A, T);
		vebInsert(A, i);
	}
}

inline void vebErase(VebSub& T, uint32_t x)
{
	if (empty(T) || x >= T.M) return;
	if (T.M <= bitSize) {
		unset(T.p, x);
		return;
	}
	uint32_t lo = low(T);
	uint32_t hi = high(T);
	if (x < lo || x > hi) return;
	if (lo == hi && x == lo) {
		sethigh(T, 0);
		setlow(T, 1);
		return;
	}
	uint32_t i, j;
	VebSub B, A;
	aux(A, T);
	if (x == lo) {
		if (empty(A)) {
			setlow(T, hi);
			return;
		} else {
			i = low(A);
			branch(B, T, i);
			j = low(B);
			setlow(T, i * ipow(T.k / 2) + j);
		}
	} else if (x == hi) {
		if (empty(A)) {
			sethigh(T, lo);
			return;
		} else {
			i = high(A);
			branch(B, T, i);
			j = high(B);
			sethigh(T, i * ipow(T.k / 2) + j);
		}
	} else {
		i = highbits(x, T.k / 2);
		j = lowbits(x, T.k / 2);
		branch(B, T, i);
	}
	vebErase(B, j);
	if (empty(B)) vebErase(A, i);
}

inline void mkfull(VebSub& T)
{
	if (T.M <= bitSize) {
		encode(T.p, bytes(T.M), ones(T.M));
		return;
	}
	setlow(T, 0);
	sethigh(T, T.M - 1);
	VebSub A;
	aux(A, T);
	mkfull(A);
	uint32_t m = highbits(T.M - 1, T.k / 2) + 1;
	for (uint32_t i = 0; i < m; i++) {
		VebSub B;
		branch(B, T, i);
		mkfull(B);
		if (i == 0) vebErase(B, 0);
		if (i == m - 1) vebErase(B, lowbits(T.M - 1, T.k / 2));
	}
}

inline uint32_t vebFindNext(const VebSub& T, uint32_t x)
{
	uint32_t hi = high(T);
	if (empty(T) || x > hi) return T.M;
	if (T.M <= bitSize) {
		uint32_t y = decode(T.p, bytes(T.M));
		y &= zeros(x);
		if (y > 0) return cybozu::bsf(y);
		return T.M;
	}
	uint32_t lo = low(T);
	if (x <= lo) return lo;
	VebSub A;
	aux(A, T);
	if (empty(A) || x == hi) return hi;
	uint32_t i = highbits(x, T.k / 2);
	uint32_t j = lowbits(x, T.k / 2);
	VebSub B;
	branch(B, T, i);
	if (!empty(B) && j <= high(B)) {
		return i * ipow(T.k / 2) + vebFindNext(B, j);
	}
	i = vebFindNext(A, i + 1);
	if (i == A.M) return hi;
	branch(B, T, i);
	return i * ipow(T.k / 2) + low(B);
}

inline uint32_t vebFindPrev(const VebSub& T, uint32_t x)
{
	uint32_t lo = low(T);
	if (empty(T) || x < lo || x >= T.M) return T.M;
	if (T.M <= bitSize) {
		uint32_t y = decode(T.p, bytes(T.M));
		y &= ones(x + 1);
		if (y > 0) return cybozu::bsr(y);
		return T.M;
	}
	uint32_t hi = high(T);
	if (x >= hi) return hi;
	VebSub A;
	aux(A, T);
	if (empty(A) || x == lo) return lo;
	uint32_t i = highbits(x, T.k / 2);
	uint32_t j = lowbits(x, T.k / 2);
	VebSub B;
	branch(B, T, i);
	if (!empty(B) && j >= low(B)) {
		return i * ipow(T.k / 2) + vebFindPrev(B, j);
	}
	i = vebFindPrev(A, i - 1);
	if (i == A.M) return lo;
	branch(B, T, i);
	return i * ipow(T.k / 2) + high(B);
}

inline void vebCreate(VebSub& T, uint32_t M, bool full)
{
	assert(M > 1);
	T.k = fls(M-1);
	T.p = (uint8_t*)malloc(vebsize(M));
	T.M = M;
	if (full) {
		mkfull(T);
	} else {
		mkempty(T);
	}
}

} // veb_tree_local

class VebTree {
	veb_tree_local::VebSub v;
	VebTree(const VebTree&);
	void operator=(const VebTree&);
public:
	VebTree()
	{
		v.p = 0;
	}
	explicit VebTree(uint32_t M, bool full = false)
	{
		veb_tree_local::vebCreate(v, M, full);
	}
	~VebTree()
	{
		free(v.p);
	}
	uint32_t findNext(uint32_t x) const
	{
		return veb_tree_local::vebFindNext(v, x);
	}
	bool has(uint32_t x) const { return findNext(x) == x; }
	uint32_t findPrev(uint32_t x) const
	{
		return veb_tree_local::vebFindPrev(v, x);
	}
	void insert(uint32_t x)
	{
		veb_tree_local::vebInsert(v, x);
	}
	void erase(uint32_t x)
	{
		veb_tree_local::vebErase(v, x);
	}
	uint32_t getM() const { return v.M; }
};
