#include <stdio.h>

template<class T>
struct X {
	static int x;
	static const char *get()
	{
		static const char *m = "abc";
		printf("m=%p %s\n", m, m);
		printf("&x=%p\n", &x);
		return m;
	}
};

template<class T>
int X<T>::x = 5;

