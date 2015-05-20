#include <stdio.h>
#include <vector>

struct Vec {
	Vec() { puts("cstr"); }
	Vec(Vec&&) { puts("move cstr"); }
	Vec(const Vec&) { puts("copy str"); }
	Vec& operator=(const Vec&) { puts("copy assign"); return *this; }
	Vec& operator=(Vec&&) { puts("move assign"); return *this; }
	~Vec() { puts("dstr"); }
};
struct X {
	Vec v;
	Vec& data() & { puts("data &"); return v; }
	Vec data() && { puts("data &&"); return std::move(v); }
	Vec data2() { puts("data2"); return v; }
	Vec data3() { puts("data3"); return std::move(v); }
};

int main()
{
	X x;
	{
		puts("1");
		Vec v = x.data();
	}
	{
		puts("2");
		auto v = x.data();
	}
	{
		puts("3");
		auto v = x.data();
	}
	{
		puts("4");
		auto v = x.data2();
	}
	{
		puts("5");
		auto v = x.data3();
	}
	{
		puts("1a");
		Vec v = X().data();
	}
	{
		puts("2a");
		auto v = X().data();
	}
	{
		puts("3a");
		auto v = X().data();
	}
	{
		puts("4a");
		auto v = X().data2();
	}
	{
		puts("5a");
		auto v = X().data3();
	}
}
