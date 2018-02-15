# 並列(parallel)プログラミング

## 目的

C++標準ライブラリに実際に普及している並列プログラミングを導入する。

## 実行ポリシー

並列アルゴリズムは`ExecutionPolicy`という名前のテンプレート引数を持つ関数テンプレートである。

`execution`ヘッダの`std::execution::seq`, `std::execution::par`, `std::execution::par_unseq`の3種類ある。

* `seq` : 従来通り先頭から順に処理する。
* `par` : 複数スレッドを用いて並列実行される。
* `par_unseq` : `par`かつSIMD的なベクトル実行される。
    スレッドは自分で管理してSIMDだけコンパイラに任せたいことがあるので`unseq`のみも欲しいとC++WGで提案したけど却下された。

## 制約

* 特に言及されない限りイテレータの各要素(型T)は複数回コピーされる可能性がある。
* 要素のアクセスは非決定的なのでその順序に依存するコードを書くべきではない。
* ライブラリは各要素のアクセスの同期処理は行わない。データ競合が起きないようにするのはユーザの責任。

## 例

```
std::vector<int> v = ...

// 従来のソート
std::sort(v.begin(), v.end());

// シーケンシャルソート(従来のソート)を明示
sort(seq, v.begin(), v.end());

// 並列ソートを許可
sort(par, v.begin(), v.end());

// ベクトル化と並列ソートを許可
sort(par_unseq, v.begin(), v.end());

// すべての値を2倍する
std::for_each(par, v.begin(), v.end(), [](auto& x) { x *= 2; });

```

## 並列アルゴリズム

* 並列アルゴリズムは要素アクセス関数経由でオブジェクトにアクセスする。
* seqが指定されたときは呼び出しスレッドの中でシーケンシャルオーダーでアクセスする。
* parが指定されるたときは不定数のスレッドから不定の順序で実行されうる。
    * データレースやデッドロックを起こさないようにするのは呼び出し側の責任。
    * 結果の正しさを保証するのも呼び出し側の責任。

### 例

    std::vector<int> a = { 0, 1 };
    std::vector<int> v;
    for_each(par, a.begin(), a.end(),
       [&](int i) { v.push_back(i * 2 + 1); }
    );
    // vへのpush_backが非同期アクセスなのでデータレース

    int x = 0;
    std::mutex m;
    std::vector<int> a = { 0, 1 };
    for_each(par, a.begin(), a.end(),
       [&](int i) {
           m.lock();
           ++x;
           m.unlock();
       }
    );
    // 正しく x = 2 になる。

* par_unseqの場合、parに加えて単一スレッドでも複数関数オブジェクトの呼び出しのインタリーブが発生するかもしれない。
    * mutexを使うとデッドロックの危険性が高い。
* 標準関数は次のときにvectorization-unsafeという。
    * 別の関数呼び出しとの同期が明記されている
    * 同期することが明記されている関数呼び出し
    * メモリ確保、解放以外の関数

### 例

    int x = 0;
    std::mutex m;
    std::vector<int> a = { 0, 1 };
    for_each(par_unseq, a.begin(), a.end(),
       [&](int i) {
           m.lock();
           ++x;
           m.unlock();
       }
    );
    // 同じスレッドでm.lock()が呼ばれるとデッドロックを起こす。

* par, par_unseqはシステムリソースが足りないときシーケンシャルな実行になる。
* par, par_unseqはInputIteratorを受けたとしても実際にはRandomAccessIteratorだった場合、operator[]を使ってよい。
  この場合、operator[]がレースフリーであることは呼び出し側の責任。
* 処理系依存の型のポリシーを受けたときは、処理系依存。

## 並列アルゴリズムの例外

* 実行中に必要なメモリがなければstd::bad_allocを投げる。
* 要素アクセス関数が補足されなかった例外を出したときは(多分)std::terminate()が呼ばれる。

## 一般和の定義

   * g_sum(op, a_1, ..., a_N) ; 可換な和
       * return a_1 if N = 1
       * return op(g_sum(op, b_1, ..., b_k), g_sum(op, b_(k+1), ..., b_N))
       * ただし b_1, ..., b_Nはa_1, ..., a_Nの置換, 1 <= k < N


   * g_noncomm_sum(op, a_1, ..., a_N) ; 非可換な和
       * return a_1 if N = 1
       * op.(g_noncomm_sum(op, a_1, ..., a_k), g_noncomm_sum(op, a_(k+1), .., a_N)
       * ただし 1 <= k < N

## for_each
```
template<class ExecutionPolicy,
         class ForwardIterator,
         class Function>
void for_each(
  ExecutionPolicy&& exec,
  ForwardIterator first,
  ForwardIterator last,
  Function f);
```

逐次for_eachと違い並列実行させるときはFunctionはCopyConstructibleでなければならない。

## for_each_n

サイズ指定のfor_each。ExecutionPolicyなしの場合はCopyConstructibleでなくてもよいが、
ExecutionPolicyありの場合はCopyConstructibleでなければならない(他も同様)。
```
template<class ExecutionPolicy,
         class ForwardIterator,
         class Size, class Function>
ForwardIterator for_each_n(
  ExecutionPolicy&& exec,
  ForwardIterator first,
  Size n,
  Function f);
```

## reduce

順序が保証されないことを除いて標準のaccumulateと同じ。

    template<class ExecutionPolicy, class InputIterator, class T, class BinaryOperation>
    T reduce(ExecutionPolicy&& exec, InputIterator first, InputIterator last, T init, BinaryOperation op);

ただしbinary_opが非結合的や非可換の場合、結果は非決定的。
g_sum(op, init, *first, ..., *(first + (last - first) - 1))を返す。

## inclusive_scanとexclusive_scan
```
template<class ExecutionPolicy,
         class ForwardIterator1,
         class ForwardIterator2,
         class BinaryOperation>
ForwardIterator2 inclusive_scan(
  ExecutionPolicy&& exec,
  ForwardIterator1 first,
  ForwardIterator1 last,
  ForwardIterator2 result,
  BinaryOperation binary_op);
```

```
template<class ExecutionPolicy,
         class ForwardIterator1,
         class ForwardIterator2,
         class T,
         class BinaryOperation>
ForwardIterator2 exclusive_scan(
  ExecutionPolicy&& exec,
  ForwardIterator1 first,
  ForwardIterator1 last,
  ForwardIterator2 result,
  T init,
  BinaryOperation binary_op);
```

opは[first, last), [result, result + (last - first))区間の値を変更したり、iteratorを無効にしてはいけない。


### inclusive_scan
区間[result, result + (last - first))内の各iに対して
g_noncomm_sum(op, init, *first, ..., *(first + (i - result))を返す。

### exclusive_scan
区間[result, result + (last - first))内の各iに対して
g_noncomm_sum(op, init, *first, ..., *(first + (i - result) - 1))を返す。

* inclusiveとexclusiveの差はi番目の要素に対して*(first + (i - result))を含むか含まないかの違い。
* opが結合的でなければ結果は非決定的。

## Q&A

### inclusive_scanなどでoutputしたとき順序は保証されないと使えないのでは?

Outputiteratorがあるのは通常版のみ

```
template<
  class InputIterator,
  class OutputIterator,
  class BinaryOperation,
  class T>
OutputIterator inclusive_scan(
  InputIterator first,
  InputIterator last,
  OutputIterator result,
  BinaryOperation binary_op,
  T init);
```

### for_eachで関数オブジェクトは何が返るの?

並列版のfor_eachの戻り値の型はvoid。通常版と並列版で異なってることが多いので一つ一つアルゴリズムの宣言をちゃんとみないといけない。

### par_unseqの場合にどうしてロックがかかる?

たとえば
```
std::for_each(par_unseq, v.begin(), v.end(), [](auto& x) { x = x * 2 + 5; });

v = [v0 v1 v2 v3 v4 v5 v6 v7 v8 v9 va vb vc vd ve vf]
```
を4個ずつSIMDで処理する場合、`[v0 v1 v2 v3]`に対して

```
t = [v0 v1 v2 v3]
t *= [2 2 2 2] ; SIMD1命令
t += [5 5 5 5] ; SIMD1命令
```
というような処理に変換される。`[](auto& x){ m.lock(); x++; m.unlock(); }`の場合、
```
t = [v0 v1 v2 v3]
[m.lock() m.lock() m.lock() m.lock()]
t += [1 1 1 1]
```
となって、同じスレッドで複数回mをlockしようとしてデッドロックになっても文句は言えない。

### 可換な和と非可換な和が分かりにくかった。

たとえばS = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7の場合、viが整数なら順序を入れ換えて足しても結果は同じ(整数の足し算は可換である)。
だから
```
S = [v0 v1 v2 v3] + [v4 v5 v6 v7] = (v0 + v4) + (v1 + v5) + (v2 + v6) + (v3 + v7)
```
と計算してもよい。よってpar_unseqを使って高速化できる。`g_sum`はこのバージョン。

別の例として
M = M0 * M1 * M2 * M3 * M4 * M5 * M6 * M7の場合、Miが行列なら順序を変えると結果は異なる(行列A, Bに対してAB = BAとは限らない)。
したがって`par_unseq`は使えない。

けれども(A * B) * C = A * (B * C)という性質(結合律)が成り立っているので、Mを求めるときに
```
M = ((M0 * M1) * (M2 * M3)) * ((M4 * M5) * (M6 * M7))
```
という順序で計算してもよい。つまり`M0 * M1`, `M2 * M3`, `M4 * M5`, `M6 * M7`を並列に同時に計算し、次に
`(M0 * M1) * (M2 * M3)`と`(M4 * M5) * (M6 * M7)`を計算して最後に掛けるという方法をとれる。
よって`par`を使って高速化できる。`g_noncomm_sum`はこのバージョン。

まとめると、

演算の性質   |      例|使える実行ポリシー
-------------|--------|---------------------
結合律のみ   |行列の積|par
結合律+可換性|整数の和|par_unseq(parも使える)