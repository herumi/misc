/*
clang++-14 -fsanitize=memory substr_err.cpp -I ./include && ./a.out
Uninitialized bytes in __interceptor_fwrite at offset 0 inside [0x7fffe7e19520, 1)
==2393379==WARNING: MemorySanitizer: use-of-uninitialized-value
    #0 0x7f4c77339b34 in std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long) (/lib/x86_64-linux-gnu/libstdc++.so.6+0x13cb34) (BuildId: f57e02bfadacc0c923c82457d5e18e1830b5faea)
    #1 0x5568abe2d71e in main (/home/shigeo/Program/cybozulib/a.out+0xa771e) (BuildId: 905c07c201f0daa2d04a4f912ca1048ac321e39c)
    #2 0x7f4c76ef7d8f in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #3 0x7f4c76ef7e3f in __libc_start_main csu/../csu/libc-start.c:392:3
    #4 0x5568abda53f4 in _start (/home/shigeo/Program/cybozulib/a.out+0x1f3f4) (BuildId: 905c07c201f0daa2d04a4f912ca1048ac321e39c)

SUMMARY: MemorySanitizer: use-of-uninitialized-value (/lib/x86_64-linux-gnu/libstdc++.so.6+0x13cb34) (BuildId: f57e02bfadacc0c923c82457d5e18e1830b5faea) in std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
Exiting
*/
#include <string>
#include <iostream>

int main()
{
	std::string ret = "a.out";
	size_t pos = ret.find('.');
	std::string sub = ret.substr(0, pos);
	std::cout << "sub=" << sub << std::endl;
//	printf("sub=%s\n", sub.c_str());
}
