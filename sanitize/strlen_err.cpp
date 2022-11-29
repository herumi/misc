/*
% clang++-14 -fsanitize=memory strlen_err.cpp && ./a.out
Uninitialized bytes in __interceptor_strlen at offset 3 inside [0x7ffe5e364c80, 4)
==2410840==WARNING: MemorySanitizer: use-of-uninitialized-value
    #0 0x5645a430f4f1 in main (/home/shigeo/Program/misc/sanitize/a.out+0xa74f1) (BuildId: 0e82596b93e05105f23ff5278d7ca0799515bf3e)
    #1 0x7fc49aa86d8f in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #2 0x7fc49aa86e3f in __libc_start_main csu/../csu/libc-start.c:392:3
    #3 0x5645a4287304 in _start (/home/shigeo/Program/misc/sanitize/a.out+0x1f304) (BuildId: 0e82596b93e05105f23ff5278d7ca0799515bf3e)

SUMMARY: MemorySanitizer: use-of-uninitialized-value (/home/shigeo/Program/misc/sanitize/a.out+0xa74f1) (BuildId: 0e82596b93e05105f23ff5278d7ca0799515bf3e) in main
Exiting
*/
#include <string>
#include <stdio.h>
#include <string.h>

int main()
{
	printf("len=%zd\n", strlen(std::string("abc").c_str()));
}
