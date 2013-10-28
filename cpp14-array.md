# 配列の拡張

## 参照

* core [N3639 Runtime-sized arrays with automatic storage duration](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3639.html)
* lib [N3662 C++ Dynamic Arrays](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3662.html)
* 全体 [N3820 Working Draft, Technical Specification - Array Extensions](http://isocpp.org/files/papers/n3820.html)

細かいところで上記に書かれていることが違っていたりする(配列が0サイズを許すか否かなど→許す方向に)。
最終的な規格がどう決まるか注意が必要。

## core
coreは実行時に長さが確定する配列(array of runtime bound : 以降ARBと略す)の機能を提供する。
これはC99のVLA(variable length array)と似ているが同じものではない。

    void f(int n)
    {
        int a[n];
    }

### ARBの特徴

* literal型でない
* pointer型に変換できる
* initializer list対応
* template parameterになれない
* copyキャプチャされない
* 参照キャプチャされたときはもとのサイズ情報を保持する
* typeid不可
* decltype(ARB)不可
* ARBの参照不可

### VLAとの違い(N3639)

ARBは
* トップ以外のサイズが実行時に決まるような多次元配列の非サポート
* 関数宣言の文法を変えない
** void f(int n, int a[n][n]);はC99ではOKだがC++では駄目
* sizeof禁止
* typedef int a[n];の禁止

## std::dynarray<T>

実行時にサイズを決められるstd::arrayのようなもの。
可能ならstack上に配列を構築する。無理ならheapにとる配列のクラス。
領域は連続して確保される。

    std::dynarray<T> a(n);のとき
    a.data() + i == &a[0] + i for 0 <= i < n

* Tはcopy cstrを持たなければならない
* T&&のcstrを持たない
* swapは持たない
* data()メソッドでT*を取得できる
* size=0は可能。ただしdata()の値はundefined
* resizeできない
* オブジェクトがstackにとられたかheapにとられたか知る方法を提供しない
* fill()を持つ

## VLAとARBとdynarrayの比較
| 項目|C99のVLA|ARB|dynarray|
|:----|--------|---|-------:|
| sizeof|実行時のsizeof可能|エラー|エラー|意味がない|
| size=0|undefined(6.7.5.2の5)|N3639ではエラー N3820ではOK|N3662ではエラー N3820ではOK|
| size<0|undefined(6.7.5.2とp.506)|N3820 ill-formed|-|
| メモリ確保できないとき|?|undefined. bad_array_lengthをthrowするのが望ましい|bad_array_length|

(注意)サイズの正負はsize_tにキャスとする前の値で決められる。

## コンパイラの対応状況

### ARB
gcc 4.8.1, clang 3.4は対応している。ただしVLAの挙動に近い。
sizeofやtypedefが可能。値を大きくとってもbad_array_lengthは投げられない。オブジェクトは生成されるが実際に値を書き込みにいったところでSEGV。
gccには-fstack-checkオプションはあるが、これには役に立たない。
VC2013は非対応。

### dynarray
gcc, clang, VCどれも非対応。
