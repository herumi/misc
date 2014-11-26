# string_view

* [N3762 string_view: a non-owning reference to a string, revision 5](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3762.html)

## 概要

大抵のC++プログラムでは文字列の参照を多く扱う。
たとえばf(const std::string&)という関数では、f("abc")という形で呼び出すと一時的にstd::stringが作られてしまう。
それを避けるためにf(const char *p, size_t len)という形にするとコードの読みやすさや安全性が損なわれる。
f()をtemplate関数にしてヘッダに書くと自由度はあがえるがコンパイルタイムやコードサイズが増える。

GoogleやLLVMではそれぞれ文字列参照の型を用意している。それらと同様の機能を持つ型を提供する。

## 名前

いろいろな候補があったがstring_viewとなった。

## std::stringに対する差異

std::stringに対する差異は可能な限り最小にしたい。
そのルールを破ってでも変更する価値のあるものについて述べる。

### remove_prefix(), remove_suffix()メソッドの追加

先頭n文字、または後ろn文字を削除する。
これらは非メンバ関数としても実装できたが、メンバ関数の方が使いやすいと思われる。

### find*()系メソッドを削除せよ(却下)

削除すべきだという意見があったが保持する。

### mutableな型を用意せよ(却下)

basic_string_view<char>というmutableな型を用意すべきとあったが、実際に必要になる場面は少ないと思われる。

### boolへの明示的な変換を提供せよ

!empty()の省略形として追加すべきという意見があったが却下された。

### strlen()の利用を避けよ(却下)

    template<size_t N>
    basic_string_view(const charT (&str)[N])

というコンストラクタを用意すべきという意見があったが却下された。

snprintfにより文字列を作ったときに問題を起こす。

    char space[PATH_MAX];
    snprintf(space, sizeof(space), "some string");
    string_view str(space);

この例では最後に'\0'が含まれる文字列となり意図しない挙動となる。

    const char *p = "abc";
    string_view str(p, strlen(p));

というコードでstrlen(p)は最適化により3となるのでパフォーマンスの問題はない(VC, gcc, clang)。

### 要素の比較ではなく(begin, end)で比較せよ(却下)

要素の比較をすると、参照中にもとの領域の文字が変わっってしまうリスクはあるが、
同じ値の文字は同じであって欲しいので中身を比較する。

### contiguous_range<charT>を待てばよいのでは(却下)

contiguous_range<charT>は似た機能を提供するがstd::stringとは意味が違う。

### string_viewを0終端せよ(却下)

0終端させるとsubstr()がcopyなしに実装できなくなってしまう。それでは意味がない。

### remove_prefixをpop_frontという名前にせよ(却下)

### string_view_(begin, end)型の提供(却下)

変換が連続領域に基づくものなら許可したいが難しい。
明示的に変換してからコンストラクタに渡したほうがいい。

## 以下実装における細かい注意について

templateクラスの形

    template<class charT, class traits = char_traits<charT>>
    class basic_string_view {};

### 非メンバ関数

* 比較関数のoperator==(), !=, <, >, <=, >=を提供する。
引数は値渡し。

* basic_stringへの変換to_string()を提供する。
  allocatorを変更したい場合はstd::string(str.begin(), str.end(), allocator);とせよ。
* operator<<()を提供する。
* hashに対応する。

### typedef const_iterator iterator;

string_viewはconst参照のみを扱うのでiteratorはconst_iteratorと同じである。

### static constexpr size_type npos = size_type(-1);

std::string::nposとは異なる型になる可能性はあるのでstring_view::nposを使うべき。

### basic_string_view(const charT* str);

char_traits::length()にconstexprがついてないのでこれにもついてない。

### constexpr basic_string_view(const charT* str, size_type len);

宙ぶらりんの参照(dangling reference)になる可能性があるのでinitializer_listを提供しない。

### 右辺値参照のコンストラクタは提供しない

### begin()はconstexprだがrbegin()はconst_iteratorでない

今のところreverse_iteratorがリテラル型ではないため。

### constexpr const_reference operator[](size_type pos) const

非const版は用意しない。
std::stringと異なりoperator[](size())は未定義である。

### constexpr const charT* data() const noexcept;

std::string::data()と異なり0終端されるとは限らない。
この戻り値をconst charT*の引数だけを持つ関数に渡してはならない。

### void remove_prefix(size_type n);

先頭n文字を削除する。O(1)で実行される。

### void remove_suffix(size_type n);

後n文字を削除する。O(1)で実行される。

### constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const;

部分文字列を切り出す。O(1)で実行される。
