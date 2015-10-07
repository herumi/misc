#include <stdio.h>
#include <vector>
#include <list>

struct A {
	A() {}
	A(const A&) { puts("cstr:const A&"); }
	A(A&&) { puts("cstr:A&&"); }
	A(const char*) { puts("cstr:const char*"); }
	A& operator=(const A&) { puts("=:const A&"); return *this; }
	A& operator=(A&&) { puts("=:A&&"); return *this; }
	A& operator=(const char*) { puts("=:const char*"); return *this; }
	~A() { puts("dstr"); }
};

int main()
{
	using Vec = std::vector<A>;
	puts("--------------");
	puts("empty push_back");
	{
		Vec v;
		v.push_back("abc");
	}
	puts("--------------");
	puts("empty emplace_back");
	{
		Vec v;
		v.emplace_back("abc");
	}
		puts("--------------");
	{
		Vec v(1);
		puts("--------------");
		puts("non empty push_back");
		v.insert(v.begin(), "abc");
		puts("--------------");
	}
		puts("--------------");
	{
		Vec v(1);
		puts("--------------");
		puts("non empty emplace_back");
		v.emplace(v.begin(), "abc");
		puts("--------------");
	}
	puts("-------------- list --------------------");
	using List = std::list<A>;
	puts("--------------");
	puts("empty push_back");
	{
		List ls;
		ls.push_back("abc");
	}
	puts("--------------");
	puts("empty emplace_back");
	{
		List ls;
		ls.emplace_back("abc");
	}
		puts("--------------");
	{
		List ls(1);
		puts("--------------");
		puts("non empty push_back");
		ls.insert(ls.begin(), "abc");
		puts("--------------");
	}
		puts("--------------");
	{
		List ls(1);
		puts("--------------");
		puts("non empty emplace_back");
		ls.emplace(ls.begin(), "abc");
		puts("--------------");
	}
}
