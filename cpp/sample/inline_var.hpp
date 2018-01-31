#include <stdio.h>

static inline int aaa = 3;
inline int bbb = 5;

template<class T = int>
struct X {
	static int aaa;
};

template<class T>
int X<T>::aaa = 8;

