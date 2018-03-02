# 並列(parallel)プログラミング

## 目的

C++17標準ライブラリに、実際に普及している並列プログラミングを導入する。

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

## [Technical Specification for C++ Extensions for Parallelism Version 2](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4725.html)
改定案

## 名前空間とヘッダ

```
#include <meow>
std::experimental::parallelism_v2
```

## 追加される実行ポリシー
* unsequenced_policy `unseq`
* vector_policy `vec`

## `unseq`
* 並列アルゴリズムはベクトル化されうる
* 単一スレッド上で複数データ項目を操作するSIMD的な実行
* この実行ポリシーで要素アクセス関数の呼び出しは順不同で呼ばれうる
* つまり単一スレッドで複数の関数オブジェクトの呼び出しがインターリーブされる可能性がある(通常のC++の規格ではない)
* キャッチされない例外が発生すればterminate()が呼ばれる

## `vec`
* 並列アルゴリズムはベクトル化されうる
* 波面アプリケーション(wavefront application:以下wf-appと略記)の優先順位づけ(sequencing)制約に従う
* unsequenced_policyより強い保証
* wf-app制約の元で要素アクセス関数は呼び出しスレッドの中で順序づけられない方法で実行されうる
* キャッチされない例外が発生すればterminate()が呼ばれる

## 並列例外
`exception_list`は`exception_ptr`の列を保持する
```
namespace std::experimental {
inline namespace parallelism_v2 {

  class exception_list : public exception
  {
    public:
      using iterator = unspecified;

      size_t size() const noexcept;
      iterator begin() const noexcept;
      iterator end() const noexcept;

      const char* what() const noexcept override;
  };
}
}
```

## 波面アプリケーション
評価とは、値の計算、式の副作用、ステートメントの実行である。

評価Aが評価Bを含むとは次のときにいう。
* AとBは潜在的に並列でない
* Aの開始がBの開始(を伴う)、またはAの開始はBの開始前に順序づけられる
* Bの完了はAの完了(を伴う)、またはBの完了がAの完了の前に順序づけられる

これは関数呼び出し時に発生する評価を含む。

* AがBの前に決定的に順序づけられる(sequenced before)とき、
評価Aが評価Bの前に順序づけられた(ordered before)という。
この関係は推移的である(ここだけの記号でA<Bと書くことにする)。

* 注意:AがBについて非決定的に順序づけられている、またはBがunsequencedなら
AはBの前に順序づけられていないし、BもAの前に順序づけられていない。

A<Bなる評価Aについて(要素アクセス関数の々呼び出しに含まれる両方とも?)、
AがBの垂直前提(vertical antecedent)であるとは、次のときをいう。
* 次の評価Sが存在する
    * Sは評価Aを含む
    * Sは、C<AかつC<Bである全ての評価Cを含む
    * SはBを含まない
* 制御が以下のいずれも実行せずにAからBに達した
    * Sの外のステートメントにジャンプするgotoステートメントかasm宣言
    * ネストされた選択または反復statementのサブステートメントに制御を移す、S内で実行されるswitchステートメント
    * (たとえキャッチされたとしても)例外
    * longjmp

note
* 垂直前提は反射律、対称律、推移律を満たさない。
* 非公式にはAがBの直前に順序づけられているか、AがBの直前に0個以上ステートメントが入れ子になってるときAがBの垂直前提である。
* Xi, Xjは入力シーケンスのi番目, j番目の要素に対応する要素アクセス関数のアプリケーションに含まれる々式または文の評価を参照する。
* 式または文が要素アクセス関数内のループに現れるなら,
アプリケーションkの単一の式やステートメントの評価Xk, Ykがいくつかあるかもしれない。

同じ表現の2個の評価に対し、次を満たすときに水平一致するという
* どちらも要素アクセス関数のそれぞれのアプリケーションにおける最初の評価である。
* 評価BiとBjのそれぞれの垂直前提であるような水平一致評価Ai, Ajが存在する。

水平一致は同値関係。

水平マッチにより要素アクセス関数の異なるアプリケーションでの評価の間に理論的なロックステップ関係が確立する。

fを一連の引数リスト内の各引数リストに対して呼び出される関数とする。

fの波面アプリケーションとは次を満たすものをいう
* i < jならBjiの評価の前に評価Aiが順序づけられる
* Aiはある評価Biの前に順序づけられ、かつBiはBjと水平一致である
* Aiはある評価Ajと水平一致であり、AjはBjの前に順序づけられる

Note
* 波面アプリケーションは、並列アプリケーションiとjがアプリケーションjの進捗がiより先に進まないように実行する。
* AiとBiの間、AjとBjの間の関係はsequenced beforeであって垂直前提ではない。

## reduction
```
template<class T, class BinaryOperation>
  unspecified reduction(T& var, const T& identity, BinaryOperation combiner);
template<class T>
  unspecified reduction_plus(T& var);
template<class T>
  unspecified reduction_multiplies(T& var);
template<class T>
  unspecified reduction_bit_and(T& var);
template<class T>
  unspecified reduction_bit_or(T& var);
template<class T>
  unspecified reduction_bit_xor(T& var);
template<class T>
  unspecified reduction_min(T& var);
template<class T>
  unspecified reduction_max(T& var);
```
このパラグラフの各関数テンプレートは未規定型のreductionオブジェクトを返す。
その型はreduction値型を持ち、reductionのための単位元をカプセル化したもの、
combiner関数オブジェクト、初期値が得られ、最終値が格納されるlive-outオブジェクトを含む。

* アルゴリズムはreduction値型のアキュムレータとして知られる不特定多数のインスタンスが割り当てられたreductionオブジェクトを返す。
* たとえば実装はprivateなスレッドプールの各スレッドに対してアキュムレータを割り当てるかもしれない。
* 各アキュムレータはreduction単位元で初期化される。
呼び出し側で初期化されたlive-outオブジェクトがアキュムレータの一つを含むときは除外される。
* アルゴリズムはアキュムレータへの参照を要素アクセス関数の各アプリケーションに渡し、2個の同時実行呼び出しが同じアキュムレータを共有しないようにする。
アキュムレータは同時に実行されない2個のアプリケーション間で共有できるが、初期化はアキュムレータごとに一度だけ実行される。
* 要素アクセス関数の適用によるアキュムレータへの変更は部分的な結果として生じる。
* アルゴリズムが戻るある時点で、部分的な結果は単一の値になるまでreductionオブジェクトのcombiner操作を用いて一度に2個ずつ結合されて、live-outオブジェクトに戻される。
有用な結果を得るためにはアキュムレータに対する変更はcombiner操作に密接に関連する可換な操作に限定すべきである。
例えばcombiner`plus<T>`はアキュムレータを増やす操作と整合性があるが、値を倍にしたり代入したりするものはそうではない。

```
template<class T, class BinaryOperation>
unspecified reduction(T& var, const T& identity, BinaryOperation combiner);
```

* 要件
    Tは`CopyConstructible`か`MoveAssignable`である。`var = combiner(var, var)`が適切に処理されるべきである。
* 戻り値
reduction値型`T`, reduction単位元, combiner関数オブジェクト, `var`によって参照されるlive-outオブジェクトを持つ未規定のreductionオブジェクト.

### reductin単位元の例

関数                |単位元|combiner操作|
--------------------|------|------------|
reduction_plus      |T()   | x + y      |
reduction_multiplies|T(1)  | x * y      |
reduction_min       |var   | min(x, y)  |

### 例

```
extern int n;
extern float x[], y[], a;
float s = 0;
for_loop(execution::vec, 0, n,
    reduction(s, 0.0f, plus<>()),
    [&](int i, float& accum) {
        y[i] += a * x[i];
        accum += y[i] * y[i];
    }
);
```
## Inductions

## For loop

## No vec
```
template<class F>
auto no_vec(F&& f) noexcept -> decltype(std::forward<F>(f)());
```
`vector_policy`を使った並列アルゴリズムの中で要素アクセス関数の呼び出しを変える。
2個の`no_vec`の呼び出しは逐次的に処理される。
入力シーケンスSの要素関数の波面アプリケーション内の水平一致になり、
S内のある1要素に対するアプリ内のfの実行はSの別の要素のfの実行の前にsequencedされる。

例

```
extern int* p;
for_loop(vec, 0, n, [&](int i) {
  y[i] += y[i+1]; // 依存があるのに大丈夫?
  if(y[i] < 0) {
    no_vec([]{
      *p++ = i; // ここだけは`seq`と同様に処理される
    });
  }
});
```

## Task block
[P0155R0 | Task Block R5](www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0155r0.pdf)
タスクブロックは並列タスクをforkしたりjoinしたりする。

### 例
各ノード`n`に対して`compute(n)`を計算し、結果の和を得る。
```
template<typename Func>
int traverse(node *n, Func&& compute)
{
  int left = 0, right = 0;
  define_task_block([&](task_block& tb) {
    if (n->left)
      tb.run([&] { left = traverse(n->left, compute); });
    if (n->right)
      tb.run([&] { right = traverse(n->right, compute); });
  });
  return compute(n) + left + right;
}
```

### `defne_task_block`

```
namespace std::experimental {
inline namespace parallelism_v2 {
  class task_cancelled_exception;

  class task_block;

  template<class F>
    void define_task_block(F&& f);

  template<class f>
    void define_task_block_restore_thread(F&& f);
}
}
```

* `task_block`クラスの`run`メンバ関数は呼び出し側に関して(プロセスが)並列に実行されるタスクを生成する。
* `define_task_block`を使って、その生成されるタスクの呼び出しを含むプログラムコードを記述する。
* `define_task_block`は生成されたプロセスの終了をjoinで待つ。
* `run`は非同期に行われる。
* `define_task_block`は単一スレッドの通常の関数と同じように順序づけられる。
* `define_task_block_restore_thread`は呼び出されたスレッドと同じスレッドに戻る。

### task_blockクラス

```
namespace std::experimental {
inline namespace parallelism_v2 {

  class task_block
  {
    private:
      ~task_block();

    public:
      task_block(const task_block&) = delete;
      task_block& operator=(const task_block&) = delete;
      void operator&() const = delete;

      template<class F>
        void run(F&& f);

      void wait();
  };
}
}
```

* `task_block`はライブラリの実装以外からはconstruct, destruct, copy, moveはされない。
* `task_block`オブジェクトのアドレス取得はill-formed.
* タスクブロックとはdefine_task_block, define_task_block_restore_threadの呼び出しをいう。
* まだ完了していないタスクブロックの最新の呼び出しをtask blockがactiveという。
* このセクション以外の手段(たとえばスレッドや非同期による)によって実行するように指定されたコードはタスクブロックに囲まれていない。
そのためこのコードに渡される(またはキャプチャされる)`task_blcok`はactiveではない。
* activeでない`task_block`の任意の操作の結果は未定義。
* `task_block::run`が呼ばれると、`task_block`はactiveでない。
* `task_block::wait`はactiveな`task_block`によって生成されたタスクの完了を待つ。


### 例外ハンドリング
* 全ての`task_blcok`は関連する例外リストを持つ(生成直後はempty)。
* 例外が投げられたら例外リストに追加される。`task_canceled_exception`は追加されない。
