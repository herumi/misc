--- test6 ---
clang-8 -fPIC -shared sub.c -o sub.so
clang-8 main-dl.c -ldl
./a.out
main-dl
malloc=0x4005f0, free=0x4005b0
dlsym
call
sub_free
malloc=0x4005f0, free=0x4005b0 in sub
dlclose
--- test7 ---
clang-8 -fPIC -shared sub.c -o sub.so -fsanitize=address
clang-8 main-dl.c -ldl -fsanitize=address
./a.out
main-dl
malloc=0x4c6210, free=0x4c5eb0
dlsym
call
sub_free
malloc=0x4c6210, free=0x4c5eb0 in sub
dlclose
--- test8 ---
clang-8 -fPIC -shared sub.c -o sub.so -fsanitize=address
clang-8 main-dl.c -ldl -fsanitize=address -DDEEP
./a.out
==73389==You are trying to dlopen a ./sub.so shared library with RTLD_DEEPBIND flag which is incompatibe with sanitizer runtime (see https://github.com/google/sanitizers/issues/611 for details). If you want to run ./sub.so library under sanitizers please remove RTLD_DEEPBIND from dlopen flags.
Makefile:38: recipe for target 'test8' failed
make: *** [test8] Error 1
