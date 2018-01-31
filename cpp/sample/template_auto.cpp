#include <iostream>

template<class T, T n>
void f() { printf("size=%zd %d\n", sizeof(n), n); }

#ifdef CPP17
template<auto n>
void g() { printf("size=%zd %d\n", sizeof(n), n); }
#endif

int main()
{
	f<int, 1>();
	f<char, 'A'>();
#ifdef CPP17
	g<1>();
	g<'A'>();
#endif
}
