/*
	compare between constant multiplication and bit shift
	VC2012 same code(use imul)
	gcc 4.6.3 hash64_2 is slower than hash64 @core i3
	cl /Oy /Ox /Ob2 /GS-
	g++ -fno-operator-names -O3 -fno-operator-names -march=native
	VC2010
x=     80414051700, 6.55clk
x=     80414051700, 6.58clk
x=-8876417460280119952, 6.54clk
x=-8876417460280119952, 6.55clk

	gcc 4.6.3
x=     80414051700, 6.58clk
x=     80414051700, 6.53clk
x=-8876417460280119952, 6.62clk
x=-8876417460280119952, 15.97clk
*/
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <xbyak/xbyak_util.h>

// 32bit version
uint32_t hash32(const uint8_t *x, size_t n)
{
	uint32_t v = 2166136261;
	for (size_t i = 0; i < n; i++) {
		v ^= x[i];
		v *= 16777619;
	}
	return v;
}

// 64bit version
uint64_t hash64(const uint8_t *x, size_t n)
{
	uint64_t v = 14695981039346656037ULL;
	for (size_t i = 0; i < n; i++) {
		v ^= x[i];
		v *= 1099511628211ULL;
	}
	v ^= v >> 32;
	return v;
}

uint32_t hash32_2(const uint8_t *x, size_t n)
{
	uint32_t v = 2166136261;
	for (size_t i = 0; i < n; i++) {
		v ^= x[i];
		v += (v << 1) + (v << 4) + (v << 7) + (v << 8) + (v << 24);
	}
	return v;
}

uint64_t hash64_2(const uint8_t *x, size_t n)
{
	uint64_t v = 14695981039346656037ULL;
	for (size_t i = 0; i < n; i++) {
		v ^= x[i];
		v += (v << 1) + (v << 4) + (v << 5) + (v << 7) + (v << 8) + (v << 40);
	}
	v ^= v >> 32;
	return v;
}

template<class F>
void test(const std::string& s, F f)
{
	const int M = 100;
	uint64_t x = 0;
	Xbyak::util::Clock clk;
	for (int i = 0; i < M; i++) {
		clk.begin();
		x += f((const uint8_t*)s.c_str(), s.size());
		clk.end();
	}
	printf("x=%16lld, %.2fclk\n", (long long)x, clk.getClock() / double(M) / s.size());
}

int main()
{
	std::string s;
	const int N = 100000;
	s.resize(N);
	for (int i = 0; i < N; i++) {
		s[i] = (char)i;
	}
	test(s, hash32);
	test(s, hash32_2);
	test(s, hash64);
	test(s, hash64_2);
}
