clang-cl /c /O2 /DNDEBUG bitint_if64.ll
cl /O2 /DNDEBUG /DNOMINMAX /EHsc bitint_llvm_test.cpp bitint_if64.obj -I ../../mcl/include -I ../../cybozulib_ext/include /link /LIBPATH:../../cybozulib_ext/lib