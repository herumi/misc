/*
	g++ std_function_gcc_bug.cpp -O2 -std=c++11 && ./a.out
*/
#include <stdio.h>
#include <functional>
#include <stdexcept>

void f() noexcept
{
    puts("f");
}

void g()
{
    puts("g");
    throw std::runtime_error("err");
}

int main()
{
    std::function<void()> a = f;
    a();
    try{
        a();
    } catch (...) {
    }
    std::function<void()> b = g;
    try {
        b();
    } catch (...) {
    }
}
