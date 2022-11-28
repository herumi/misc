/*
% clang++-14 -fsanitize=memory list_err.cpp && ./a.out
BEGIN
==2393521==WARNING: MemorySanitizer: use-of-uninitialized-value
    #0 0x559799526280 in std::__cxx11::_List_base<int, std::allocator<int> >::_M_clear() (./a.out+0xa9280) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec)
    #1 0x5597995260e4 in std::__cxx11::_List_base<int, std::allocator<int> >::~_List_base() (./a.out+0xa90e4) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec)
    #2 0x5597995247c1 in std::__cxx11::list<int, std::allocator<int> >::~list() (./a.out+0xa77c1) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec)
    #3 0x559799524611 in X::~X() (./a.out+0xa7611) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec)
    #4 0x559799524485 in main (./a.out+0xa7485) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec)
    #5 0x7f7fadab8d8f in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #6 0x7f7fadab8e3f in __libc_start_main csu/../csu/libc-start.c:392:3
    #7 0x55979949c304 in _start (./a.out+0x1f304) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec)

SUMMARY: MemorySanitizer: use-of-uninitialized-value (./a.out+0xa9280) (BuildId: 02ad193d3a85c86c7bfedec6c4168add104c49ec) in std::__cxx11::_List_base<int, std::allocator<int> >::_M_clear()
Exiting
*/
#include <list>
#include <stdio.h>

struct X {
  std::list<int> v;
  X()
  {
    v.push_back(0);
  }
};


int main()
{
  puts("BEGIN");
  {
    X x;
  }
  puts("END");
}


