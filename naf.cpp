#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <assert.h>

template<class Vec>
void convBin(Vec& v, int64_t z)
{
	v.clear();
	while (z > 0) {
		v.insert(v.begin(), (z & 1) ? 1 : 0);
		z /= 2;
	}
}

template<class T>
void put(const T& x, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		printf("%2d,", x[i]);
	}
	printf("\n");
}
template<class T>
void put(const T& x)
{
	put(x, x.size());
}
template<class Vec>
size_t getContinuousVal(const Vec& v, size_t pos, int val)
{
	while (pos >= 2) {
		if (v[pos] != val) break;
		pos--;
	}
	return pos;
}
template<class Vec>
void convertToNAF(Vec& v, const Vec& in)
{
	v = in;
	size_t pos = v.size() - 1;
	for (;;) {
		size_t p = getContinuousVal(v, pos, 0);
		if (p == 1) return;
		assert(v[p] == 1);
		size_t q = getContinuousVal(v, p, 1);
		if (q == 1) return;
		assert(v[q] == 0);
		if (p - q <= 1) {
			pos = p - 1;
			continue;
		}
		v[q] = 1;
		for (size_t i = q + 1; i < p; i++) {
			v[i] = 0;
		}
		v[p] = -1;
		pos = q;
	}
}

template<class Vec>
void convNaf(Vec& v, const Vec& in)
{
	v.resize(in.size());
	int c = 0;
	for (size_t i = in.size() - 1; i != size_t(-1); i--) {
		int d = c + in[i];
		if (i > 0) d += in[i - 1];
		d /= 2;
		v[i] = c + in[i] - 2 * d;
		c = d;
	}
	if (c > 0) v.insert(v.begin(), c);
}

template<class Vec>
int64_t getVal(const Vec& v)
{
	int64_t z = 0;
	const size_t n = v.size();
	for (size_t i = 0; i < n; i++) {
		z += (int64_t(1) << i) * v[n - 1 - i];
	}
	return z;
}
int main()
{
	typedef std::vector<int8_t> SignVec;
	uint64_t z = 4965661367192848881;
	SignVec bin, naf, ok, naf2;
	convBin(bin, z);
	std::cout << "z  =" << z << std::endl;
	std::cout << "z  =" << getVal(bin) << std::endl;
	put(bin);

	ok = {1, 0, 0, 0, 1, 0, 1, 0, 0, -1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1};
	put(ok);
	std::cout << "ok =" << getVal(ok) << std::endl;
	convertToNAF(naf2, bin);
	puts("naf2");
	put(naf2);
	convNaf(naf, bin);
	put(naf);
	std::cout << "naf=" << getVal(naf) << std::endl;
}
