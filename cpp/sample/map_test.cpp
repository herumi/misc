#include <stdio.h>
#include <stdlib.h>
#include <cybozu/xorshift.hpp>
#include <map>

struct G {
	int cstr;
	int copy_cstr;
	int mov_cstr;
	int copy_assign;
	int mov_assign;
	int lt1;
	int eq1;
	int lt2;
	int eq2;
	void clear()
	{
		cstr = 0;
		copy_cstr = 0;
		mov_cstr = 0;
		copy_assign = 0;
		mov_assign = 0;
		lt1 = 0;
		eq1 = 0;
		lt2 = 0;
		eq2 = 0;
	}
	void put() const
	{
		printf("cstr=%d copy_cstr=%d mov_cstr=%d\n", cstr, copy_cstr, mov_cstr);
		printf("copy_assign=%d mov_assign=%d\n", copy_assign, mov_assign);
		printf("1 lt=%d eq=%d\n", lt1, eq1);
		printf("2 lt=%d eq=%d\n", lt2, eq2);
	}
} g;

struct X {
	int x;
	explicit X(int x = 0)noexcept  : x(x) { g.cstr++; }
	X(const X& x) noexcept : x(x.x) { g.copy_cstr++; }
	X(X&& x) noexcept : x(x.x) { g.mov_cstr++; }
	void operator=(const X& rhs) noexcept { x = rhs.x; g.copy_assign++; }
	void operator=(X&& rhs) noexcept { x = rhs.x; g.mov_assign++; }
	bool operator==(const X& b) const { g.eq1++; return x == b.x; }
	bool operator<(const X& b) const { g.lt1++; return x < b.x; }

	bool operator==(int b) const { g.eq2++; return x == b; }
	bool operator<(int b) const { g.lt2++; return x < b; }
};


struct A {};

using Map = std::map<X, A, std::less<>>;

template<class M, class F>
void test(const F& f)
{
	cybozu::XorShift rg;
    M m;
	const int C = 65537;
	g.clear();
	for (int i = 0; i < 1000; i++) {
		int v = rg.get32() % C;
		f(m, v);
	}
	g.put();
}

void insert_if_absent1(Map& m, int v)
{
	m.try_emplace(X(v));
}

void insert_if_absent2(Map& m, int v)
{
    Map::iterator it = m.find(X(v));
    if (it != m.end()) return;
    m.try_emplace(X(v));
}

void insert_if_absent3(Map& m, int v)
{
    Map::iterator it = m.lower_bound(v);
    if (it != m.end() && it->first == v) {
		return;
	}
    m.emplace_hint(it, v, A());
}

struct B {
	struct A a;
	bool copy;
	B()
		: a()
		, copy(false)
	{
	}
	B(const B&) { copy = false; }
	B(B&&) { copy = false; }
	void operator=(const B&) { copy = false; }
	void operator=(B&&) { copy = false; }
};

using Map2 = std::map<X, B, std::less<>>;

void insert_if_absent4(Map2& m, int v)
{
    m.emplace(v, B());
}

int main()
{
	test<Map>(insert_if_absent1);
	test<Map>(insert_if_absent2);
	test<Map>(insert_if_absent3);
	test<Map2>(insert_if_absent4);
}
