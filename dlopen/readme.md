# test of address sanitizer for dlopen

`main.c` call a function in `sub.c`.

test|main.c|sub.c|gcc|clang|
-|-|-|-|-|
test1| |static|ok|ok|
test2| ASan|static+ASan|ok|ok|
test3| |shared|ok|ok|
test4| |shared+ASan|err|err|
tset5|LD_PRELOAD|shared+ASan|ok|ok|
test6|dlopen|sahred|ok|ok|
test7|dlopen+ASan|shared+ASan|ok|ok|
test8|dlopen+RTLD_DEEPBIND+ASan|sahred+ASan|ok|err|

- [gcc-result](gcc-result.txt)
- [clang-result](clang-result.txt)
