#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <cybozu/xorshift.hpp>
#include <cybozu/exception.hpp>
#include <cybozu/random_generator.hpp>

template<class T, class RG>
void shuffle1(T& v, RG& rg)
{
	if (v.size() <= 1) return;
	for (size_t i = 0, n = v.size(); i < n; i++) {
		size_t r = size_t(rg.get64() % n);
		std::swap(v[i], v[r]);
	}
}

struct IntVec : public std::vector<int> {
	bool operator<(const IntVec& rhs) const
	{
		if (size() != rhs.size()) {
			throw cybozu::Exception("IntVec:operator<") << size() << rhs.size();
		}
		for (size_t i = 0; i < size(); i++) {
			int a = (*this)[i];
			int b = rhs[i];
			if (a < b) return true;
			if (a > b) return false;
		}
		return false;
	}
};

typedef std::map<IntVec, int> Map;

template<class V, class RG>
void test(void f(V&, RG& rg), size_t n)
{
	cybozu::XorShift rg;
	const int N = 10000000;
	Map tbl;
	IntVec x;
	x.resize(n);
	for (int i = 0; i < N; i++) {
		for (size_t j = 0; j < n; j++) x[j] = (int)j;
		f(x, rg);
		tbl[x]++;
	}
	int verifyN = 0;
	for (Map::const_iterator i = tbl.begin(), ie = tbl.end(); i != ie; ++i) {
		const int c = i->second;
		printf("%.2f ", c * 100.0 / N);
		verifyN += c;
	}
	if (N != verifyN) {
		printf("ERR N=%d, verifyN=%d\n", N, verifyN);
		exit(1);
	}
	printf("\n");
}

int main()
{
	for (size_t i = 2; i < 6; i++) {
		printf("i=%d\n", (int)i);
		test(shuffle1<IntVec, cybozu::XorShift>, i);
		test(cybozu::shuffle<IntVec, cybozu::XorShift>, i);
	}
}
