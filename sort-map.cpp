/*

                  Windows           Linux
                 phase1   phase2   phase1   phase2
n    10000 t 0        0   150928        0   156778
n    10000 t 1        0   130942        0   141770
n    10000 t 2        0   160748        0   166585
n    10000 t 3   205306   221382   162675   231381
n    10000 t 5   205306    19999   162675    29997
n    10000 t 6   205306     9999   162675    19998
n    10000 t 7   205306     9999   162675    29997
*/
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <boost/lambda/lambda.hpp>
#include <boost/ref.hpp>
#include <stdio.h>

static struct Counter {
	int cstr_;
	int copyCstr_;
	int assign_;
	int dstr_;
	int cmp_;
	void put() const
	{
//		printf("cstr %8d copyCstr %8d assign %8d dstr %8d compare %8d\n", cstr_, copyCstr_, assign_, dstr_, cmp_);
		printf("compare %8d\n", cmp_);
	}
	void reset()
	{
		cstr_ = copyCstr_ = assign_ = dstr_ = cmp_ = 0;
	}
} s_counter;

struct A {
	int a_;
	A(int a = 0)
		: a_(a)
	{
		s_counter.cstr_++;
	}
	A(const A& x)
		: a_(x.a_)
	{
		s_counter.copyCstr_++;
	}
	const A& operator=(const A& x)
	{
		a_ = x.a_;
		s_counter.assign_++;
		return *this;
	}
	~A()
	{
		s_counter.dstr_++;
	}
	bool operator<(const A& x) const
	{
		s_counter.cmp_++;
		return a_ < x.a_;
	}
};

struct Map : public std::map<A, int> {
	void put() const
	{
		for (const_iterator i = begin(), e = end(); i != e; ++i) {
			printf("{%d, %d}, ", i->first.a_, i->second);
		}
		printf("\n");
	}
};

void test(size_t n, int type)
{
	using namespace boost::lambda;
	std::vector<A> va(n);
	std::vector<int> vi(n);
	const int putNumMax = 20;
	Map map;

//	printf("n %d t %d\n", n, type);
	int keep = 0;
	/* randomize [0, n) */
	srand(0);
	int idx = 0;
	std::generate(va.begin(), va.end(), var(idx)++);
	std::random_shuffle(va.begin(), va.end());
	std::random_shuffle(va.begin(), va.end());
	std::random_shuffle(va.begin(), va.end());

	std::generate(vi.begin(), vi.end(), var(idx)++);
	if (n < putNumMax) {
		for (size_t i = 0; i < n; i++) {
			printf("{%d, %d}, ", va[i].a_, vi[i]);
		}
		printf("\n");
	}

	s_counter.reset();
	switch (type) {
	case 0: /* map with [] */
		for (size_t i = 0; i < n; i++) {
			map[va[i]] = vi[i];
		}
		break;
	case 1: /* map with insert */
		for (size_t i = 0; i < n; i++) {
			map.insert(Map::value_type(va[i], vi[i]));
		}
		break;
	case 2: /* map with insert */
		{
			Map::iterator hint = map.insert(map.begin(), Map::value_type(va[0], vi[0]));
			for (size_t i = 1; i < n; i++) {
				hint = map.insert(++hint, Map::value_type(va[i], vi[i]));
			}
		}
		break;
	default:
	case 3: /* sort + map with [] */
	case 4: /* sort + map with insert */
	case 5: /* sort + map with insert */
		{
			/* sort first */
			typedef std::vector<const A*> VectP;
			VectP vp(n);
			int idx = 0;
			for (size_t i = 0; i < n; i++) { vp[i] = &va[i]; }
			std::sort(vp.begin(), vp.end(), *_1 < *_2);
			keep = s_counter.cmp_; s_counter.reset();
			/* already sorted */
			switch (type) {
			case 3:
				for (size_t i = 0; i < n; i++) {
					map[*(vp[i])] = vi[vp[i] - &va[0]];
				}
				break;
			case 4:
				for (size_t i = 0; i < n; i++) {
					map.insert(Map::value_type(*(vp[i]), vi[vp[i] - &va[0]]));
				}
				break;
			case 5:
			case 6:
			case 7:
				{
					int adj = type - 5;
					Map::iterator hint = map.insert(map.begin(), Map::value_type(*(vp[0]), vi[vp[0] - &va[0]]));
					for (size_t i = 1; i < n; i++) {
						std::advance(hint, adj);
						hint = map.insert(hint, Map::value_type(*(vp[i]), vi[vp[i] - &va[0]]));
					}
				}
				break;
			}
		}
		break;
	}
	if (n < putNumMax) map.put();
	printf("n %8d t %d %8d %8d\n", n, type, keep, s_counter.cmp_);
}

int main(int argc, char *argv[])
{
	int num = argc == 1 ? 16 : atoi(argv[1]);
	printf("num %d\n", num);
	for (int i = 0; i < 8; i++) {
		if (i == 4) continue;
		test(num, i);
	}
	return 0;
}
