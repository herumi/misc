/*
 clang++-14 -fsanitize=memory sanitize_memory_err.cpp && ./a.out
BEGIN
==2392082==WARNING: MemorySanitizer: use-of-uninitialized-value
    #0 0x5575e9d29040 in std::__cxx11::_List_base<X, std::allocator<X> >::_M_clear() (/home/shigeo/Program/misc/a.out+0xad040) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9)
    #1 0x5575e9d28ea4 in std::__cxx11::_List_base<X, std::allocator<X> >::~_List_base() (/home/shigeo/Program/misc/a.out+0xacea4) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9)
    #2 0x5575e9d238c1 in std::__cxx11::list<X, std::allocator<X> >::~list() (/home/shigeo/Program/misc/a.out+0xa78c1) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9)
    #3 0x5575e9d23671 in Y::~Y() (/home/shigeo/Program/misc/a.out+0xa7671) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9)
    #4 0x5575e9d23485 in main (/home/shigeo/Program/misc/a.out+0xa7485) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9)
    #5 0x7fb59410dd8f in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #6 0x7fb59410de3f in __libc_start_main csu/../csu/libc-start.c:392:3
    #7 0x5575e9c9b304 in _start (/home/shigeo/Program/misc/a.out+0x1f304) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9)

SUMMARY: MemorySanitizer: use-of-uninitialized-value (/home/shigeo/Program/misc/a.out+0xad040) (BuildId: 9a4cb4bf9fe5332db55222a25b09fdd2234a4bf9) in std::__cxx11::_List_base<X, std::allocator<X> >::_M_clear()
Exiting
*/
#include <list>
#include <unordered_map>
#include <stdio.h>

struct X {
	std::unordered_map<int, int> m_;
};

struct Y {
	std::list<X> l_;
	Y()
	{
		l_.push_back(X());
	}
};


int main()
{
	puts("BEGIN");
	{
		Y y;
	}
	puts("END");
}


