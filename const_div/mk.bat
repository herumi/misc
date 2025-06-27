@echo off
cl /EHsc /Ox /DNDEBUG div7_test.cpp div7.cpp /openmp -I ./ -I ../../mcl/include -I ../../mcl/src/