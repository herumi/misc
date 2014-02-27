@echo off
set OPT=/EHsc /Ox /Ob2 /fp:fast /Zi
echo "no avx"
cl %OPT% slow_sin.cpp && slow_sin
echo "with avx"
cl %OPT% /arch:AVX slow_sin.cpp /Feslow_sin_avx.exe && slow_sin_avx
