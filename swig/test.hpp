#include <stdio.h>

struct A {
    A(int a)
		: a(a)
	{
		puts("cstr");
	}
    ~A()
	{
		puts("dstr");
	}
    A(const A& rhs)
		: a(rhs.a)
	{
		puts("cp cstr");
	}
    A& operator=(const A& rhs)
	{
		a = rhs.a;
		puts("cp assign");
		return *this;
	}
	void addModify(int x)
	{
		puts("addModify");
		a += x;
	}
	A addReturn(int x)
	{
		puts("addReturn");
		A a(*this);
		a.a += x;
		return a;
	}
	void put() const
	{
		printf("put %d\n", a);
	}
private:
	int a;
};

