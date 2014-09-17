#define _ALLOW_KEYWORD_MACROS
#define private public
#include <stdio.h>
#include <vector>

typedef std::vector<int> IntVec;

int main()
{
	IntVec iv;
	iv.push_back(123);
	IntVec::iterator i = iv.begin();

	printf("sizeof(i)=%d\n", (int)sizeof(i));
	printf("sizeof(_Ptr)=%d\n", (int)sizeof(i._Ptr));
	printf("*_Ptr=%d\n", *i._Ptr);

	printf("ptr &*i  =%p\n", &*i);
	printf("ptr i    =%p\n", &i);
	printf("ptr proxy=%p\n", &i._Myproxy);
	printf("ptr next =%p\n", &i._Mynextiter);
	printf("ptr _Ptr =%p\n", &i._Ptr);

	printf("offset proxy=%d\n", (int)offsetof(IntVec::iterator, _Myproxy));
	printf("offset next =%d\n", (int)offsetof(IntVec::iterator, _Mynextiter));
	printf("offset _Ptr =%d\n", (int)offsetof(IntVec::iterator, _Ptr));

	printf("val proxy=%x\n", i._Myproxy);
	printf("val next =%x\n", i._Mynextiter);
	printf("val _Ptr =%x\n", i._Ptr);

}