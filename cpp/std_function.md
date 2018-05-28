# std::functionとC++17のnoexcept

C++17でnoexceptは関数の型の一部になった。
例外を投げるかもしれない関数をnoexceptつきの関数ポインタに代入できなくなった。


```
void f(){}
void g()noexcept{}

int main()
{
    void (*pf)() = f;
    void (*pg)()noexcept = g;
    pf = g;
    pg = f; // C++14まではOK. C++17でエラー
}
```

しかしstd::functionはまだnoexceptに対応していない。

```
#include <functional>
void f(){}
void g()noexcept{}

int main()
{
    std::function<void()> pf = f;
    std::function<void()noexcept> pg = g; // C++14はではOK。C++17でエラー
}
```

cf. constなインスタンスの非constメンバ関数を呼べてしまう問題

```
#include <functional>
#include <stdio.h>

struct A {
    int a = 3;
    A(){}
    int operator()() { return a++; }
};

int main()
{
    const A a;
    // a(); // エラー : constインスタンスの非constメンバ関数は呼べない
    std::function<int()> pf = a;
    printf("%d\n", pf());
    printf("%d\n", pf());
}
```
noexceptの他にconstやvolatileにも対応すべきという提案

# Qualified std::function signatures
[Qualified std::function signatures](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0045r1.pdf)

# gcc-7.3のバグ

```
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
```
```
g++ t.cpp -O2 -std=c++11 && ./a.out
```
が無限ループ
[wrong exception handling of std::function](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84858)
