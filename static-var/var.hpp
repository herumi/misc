#pragma once
#include <stdio.h>
#ifdef _MSC_VER
	#ifdef DLL_EXPORT
		#define DLL_API __declspec(dllexport)
	#else
		#define DLL_API __declspec(dllimport)
	#endif
#else
	#define DLL_API
#endif
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
	void inc()
	{
		x++;
	}
};

//DLL_API
template<class T>
int X<T>::x = 5;

extern "C" {

DLL_API void put();
DLL_API void incX();

}

struct XX {
	DLL_API static int x;
};
