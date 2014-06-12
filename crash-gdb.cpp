/*
	crash gdb sample code
	g++ crash-gdb.cpp -std=c++11 && gdb ./a.out
	clang++ crash-gdb.cpp -std=c++11 && gdb ./a.out

	g++ 4.8.2
	clang 3.5-1ubuntu1

	gdb (Ubuntu 7.7-Oubuntu3.1) 7.7 ; crash
	gdb (Ubuntu/Linaro 7.4-2012.04-0ubuntu2.1) 7.4-2012.04 ; not crash
*/
#include <utility>
#include <stdio.h>

struct BaseHolder { };

template<class Func>
struct Holder : public BaseHolder {
	Func func;
	explicit Holder(Func&& func)
		: func(std::forward<Func>(func)) { }
};

struct Runner {
	BaseHolder *holder_;
	template<class Func>
	explicit Runner(Func && func)
		: holder_(new Holder<Func>(func)) { }
	~Runner() { delete holder_; }
};

template<class T>
void f(T &)
{
	auto g =[&](){ };
	Runner{g};
}

int main()
{
	int a = 0;
	f(a);
	puts("--- ok ---");
}
