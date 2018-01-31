# C++17概略

## クラステンプレートのテンプレート引数推論

今まではstd::pairで型を明記する必要があった。

```
std::pair<int, double> p = { 234, 4.2 };
std::array<int, 4> = { 5, 9, 2, 3 };
std::mutex m;
std::lock_guard<std::mutex> al(m);
```

引数から決定されるようになった。

```
std::pair p = { 234, 4.2 };
std::array = { 5, 9, 2, 3 };
std::mutex m;
std::lock_guard al(m);
```

## 非型テンプレートパラメータのauto宣言`template<auto>`

今までは型を明記する必要があった。

```
template<class T, T n>
void f() { printf("size=%zd %d\n", sizeof(n), n); }

foo<int, 1>();
foo<char, 'A'>();
```

C++17で`template<auto>`が使えるようになった。

```
template<auto n>
void g() { printf("size=%zd %d\n", sizeof(n), n); }

g<1>();
g<'A'>();
```

## inline変数
ヘッダファイルのみでプログラム全体で実体が一つしかない変数を定義できるようになった。
今まではヘッダファイルのみで変数を作るにはtemplateのstatic変数を使ってしかできなかった。

```
template<class T = int>
struct X {
    static int aaa;
};

template<class T>
int X<T>::aaa = 8; // どの翻訳単位でも同一アドレス
```

C++17で次のように書けるようになった。

```
// ヘッダファイル
static inline int aaa = 3; // includeした翻訳単位
inline int bbb = 5; // プログラム全体で一つの変数
```

## 構造化束縛(structured bindings)

ペアや構造体などの値を分割代入できる。

```
std::set<int> s = { 1, 5, 9, 2, 4 };
auto i = s.insert(22);
if (i.second) {
   printf("inseted %d\n", *i.first);
}
```
`auto`の型は`std::pair<std::set<int>::iterator, bool>`。
これを
```
std::set s = { 1, 5, 9, 2, 4 };
auto [i, inserted] = s.insert(22);
if (inserted) {
   printf("inseted %d\n", *i);
}
```
とできる。

mapの要素を回す。

```
std::map<std::string, int> m = {
  { "abc", 3 },
  { "xyz", 4 },
  { "qqq",  2 }
};
for (const auto& [key, val] : m) {
    printf("%s %d\n", key.c_str(), val);
}
```

### 注意点

* 入れ子不可
* 使わない変数の生成抑制不可

## `if`と`switch`での初期化

`for`に合わせて`if`と`switch`の中で変数を初期化できるようになった。

今までは
```
{
    int x = func();
    if (x == NO_ERROR) {
        ...
    }
}
```
としていたのを
```
if (int x = func(); x == NO_ERROR) {
 ...
}
```
とかけるようになった。例。

```
std::set s = { 1, 5, 9, 2, 4 };
if (auto [i, inserted] = s.insert(22); inserted) {
   printf("inseted %d\n", *i);
}
```

## 並括弧初期化のauto型推論の変更

C++17までは個数に関係なく`std::initializer_list<T>`になっていた。
```
auto x1{1};
auto x2{1, 2};
```

C++17では一つはTに、複数ならエラーになった。
```
auto x1{1}; // auto = int
auto x2{1, 2}; // エラー
```

コピー初期化は今までどおり`std::initializer_list<T>`である。
```
auto x1 = {1};    // std::initializer_list<int>
auto x2 = {1, 2}; // std::initializer_list<int>
```

## 畳み込み式

可変数引数の畳み込みは今まで再帰的に書いていた。

```
const int initValue = 100;

auto sum()
{
  return initValue;
}

template<class Arg, class... Args>
auto sum(Arg head, Args... tails)
{
  return head + sum(tails...);
}
```

C++17では畳み込み式が導入された。

```
template<class... Args>
auto sumR(Args...  args)
{
    return (args + ...); // ()は必要
}
```

* `(args op ...)` ; 単項演算子 右結合 ; `p1 op (p2 op (... op pn)...)`
* `(... op args)` ; 単項演算子 左結合 ; `...((p1 op p2) op ...) op pn`
* `(args op ... op init)` ; 2項演算子 右結合 ; `p1 op (p2 op ... (pn op init))...)`
* `(init op ... op args)` ; 2項演算子 左結合 ; `(...((init op p1) op p2) op ...`

```
template<class... Args>
auto sumL(Args...  args)
{
    return (... + args);
}

template<class... Args>
auto sumRI(Args...  args)
{
    return (args + ... + initValue);
}

template<class... Args>
auto sumLI(Args...  args)
{
    return (initValue + ... + args);
}
```

* `sumR(1, 2, 3)`は`1 + (2 + 3)`
* `sumL(1, 2, 3)`は`(1 + 2) + 3`
* `sumRI(1, 2, 3)`は`1 + (2 + (3 + 100))`
* `sumLI(1, 2, 3)`は`((100 + 1) + 2) + 3`

## over-aligned型対応new

今まではalinasで指定された構造体がnewされたときアラインされているとは限らなかった。

```
struct alignas(64) X {
    uint32_t a[16]; // AVX-512用データのつもり
};

int main()
{
    X *p = new X();
    printf("p=%p\n", p);
}
```

```
% g++-5 -std=c++17 -Wall -Wextra over-aligned-new.cpp && ./a.out
p=0x220fc20 // 64バイトアライメントされていない
```
gcc 7から対応した。

```
% g++-7 -std=c++17 -Wall -Wextra over-aligned-new.cpp && ./a.out
p=0x1f5ac40
```

## クイズ1

`?`が0回か1回の後に`!`がある文字列にマッチするか。

```
#include <stdio.h>
#include <regex>

int main()
{
    std::regex r("\\??!");
    std::string str = "x!";
    std::smatch m;

    if (std::regex_search("x!", m, r)) {
        printf("match=[%s]\n", m[0].str().c_str());
    } else {
        puts("ng");
    }
}
```

```
g++ -std=c++14 test.cpp && ./a.out ; ngを表示
g++ -std=c++17 test.cpp && ./a.out ; match=[!]を表示
```

## クイズ2

```
#include <stdio.h>
#include <string>

template<class T>
struct X {
    X(T *) { puts("xxx"); }
};

template<class T, template<class T_>class Container>
struct X<Container<T> > {
    X(Container<T> *) { puts("yyy"); }
};

int main()
{
    std::string s;
    X<std::string> x(&s);
}
```

```
% g++-7 template_match.cpp -std=c++14 && ./a.out
xxx
% g++-7 template_match.cpp -std=c++17 && ./a.out
yyy
```

[DR: Matching of template template-arguments excludes compatible templates](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0522r0.html)

>This paper allows a template template-parameter to bind to a template argument whenever the template parameter is at least as specialized as the template argument. This implies that any template argument list that can legitimately be applied to the template template-parameter is also applicable to the argument template.


テンプレート引数がテンプレート引数と少なくとも同じように特殊化されているときはいつでもテンプレートテンプレート引数をテンプレート引数にバインドできるようにする。
