/*
	a sample code to crash gdb 7.7
	g++ crash-gdb.cpp -std=c++11 && gdb ./a.out
	clang++ crash-gdb.cpp -std=c++11 && gdb ./a.out

	g++ 4.6.3 -std=c++0x
	g++ 4.8.2 -std=c++11
	clang 3.5-1ubuntu1 -std=c++11

	gdb (Ubuntu/Linaro 7.4-2012.04-0ubuntu2.1) 7.4-2012.04 ; not crash
	gdb (Ubuntu 7.7-Oubuntu3.1) 7.7 ; crash
	gdb 7.8.50.20140612-cvs ; not crash
*/
#include <utility>
#include <stdio.h>

struct BaseHolder { };

template<class Func>
struct Holder : public BaseHolder {
	Func func;
	explicit Holder(Func&& func)
		: func(std::forward<Func>(func))
	{
	}
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
#ifdef __clang__
	printf("clang %d %d\n", __clang_major__, __clang_minor__);
#elif defined(__GNUC__)
	printf("gcc %d %d\n", __GNUC__, __GNUC_MINOR__);
#endif
}
