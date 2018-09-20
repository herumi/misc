## [Working Draft, C++ Extensions for Parallelism Version 2](http://www.open-std.org/jtc1/sc22/wg21/prot/14882fdis/n4757.pdf)

## Data-Parallel TYpes
### 概要
* データ並列ライブラリはデータ並列型とその操作を扱う
* データ並列型は算術型(要素型という)からなる
* データ並列型の要素数を型の幅という
* 要素単位の操作
    * 要素型のオブジェクトの各要素に指定した操作を行う
    * 各操作は互いに順序づけされない

## ` <experimental/simd>`
### simd ABIタグ

データ並列型のサイズやバイナリ表現を示す

```
namespace simd_abi {
  using scalar = see below;
  template<int N> using fixed_size = see below;
  template<class T> inline constexpr int max_fixed_size = implementation-defined;
  template<class T> using compatible = implementation-defined;
  template<class T> using native = implementation-defined;
}
```
* `scalar`は単一要素を指す
    * simd<T, simd_abi::scalar>::size() == 1
* `fixed_size<N>`はN個の要素を扱うデータ型を指す

### simd型traits
* simdのクラステンプレートであるかを示す`is_simd<T>`など

### `where_expression`
`const_where_expression`と`where_expression`はデータ並列型の要素の選択表記を抽象化したもの

```
template<class M, class T> class const_where_expression {
  const M mask; // exposition only
  T& data; // exposition only
public:
  const_where_expression(const const_where_expression&) = delete;
  const_where_expression& operator=(const const_where_expression&) = delete;
  T operator-() const && noexcept;
  T operator+() const && noexcept;
  T operator ~ () const && noexcept;
  template<class U, class Flags> void copy_to(U* mem, Flags f) const &&;
};
```

* copy_to(U* mem, Flags f)
    * Flagの種類
        * `vector_aligned_tag` ; `memory_alignment_v<T, U>`でアラインされたストレージ
        * `overaligned_tag<N>` ; `N`でアラインされたストレージ
        * `element_aligned_tag` ; `alignof(U)`でアラインされたストレージ
    * データ型dataに対して`mem[i] = static_cast<U>(data[i])`のように振る舞う

```
template<class M, class T>
class where_expression : public const_where_expression<M, T> {
public:
  template<class U> void operator=(U&& x) && noexcept;
  template<class U> void operator+=(U&& x) && noexcept;
  template<class U> void operator-=(U&& x) && noexcept;
  template<class U> void operator*=(U&& x) && noexcept;
  ...
```

* operator@@@(&& x)は`data[i] = static_cast<T>(data @@@ std::forward<U>(x))[i]`であるかのように振る舞う

## クラステンプレートsimd

```
template<class T, class Abi> class simd {
public:
  using value_type = T;
  using reference = see below;
  using mask_type = simd_mask<T, Abi>;
  using abi_type = Abi;
  static constexpr size_t size() noexcept; // simdの幅を返す

  ...
  template<class U, class Flags> copy_from(const U* mem, Flags f);
  template<class U, class Flags> copy_to(U* mem, Flags f);

  reference operator[](size_t);

  simd& operator++() noexcept;

  friend simd operator+(const simd&, const simd&) noexcept;
  ...
};
```
Abiは`simd<T, simd_abi::__gpu_y>`とか`simd<double, simd_abi::__simd_x>`とかのイメージ

### referenceはsimdやsimd_maskの要素を参照する
* `reference operator[](size_t i)`で参照する
* reference::value_typeはsimd::value_typeやsimd_mask::value_typeと同じ型
* reference型は通常の`operator@@@=`やswapがある

### simdコンストラクタ
通常のコピーコンストラクタやメモリから読み込んで設定するものがある

* `template<class U, class Flags> simd(const U* mem, Flags);`
    * Flagsはcopy_toのと同じ

### コピー関数
* `template<class U, class Flags> void copy_from(const U* mem, Flags)`
* `template<class U, class Flags> void copy_to(U* mem, Flags) const;`

### binary/unary operation, compound assignment
通常通り

### 比較関数
結果は要素ごとに比較した結果を格納する`simd_mask`型のオブジェクトになる。
```
friend mask_type operator==(const simd& lhs, const simd& rhs) noexcept;
friend mask_type operator!=(const simd& lhs, const simd& rhs) noexcept;
friend mask_type operator>=(const simd& lhs, const simd& rhs) noexcept;
friend mask_type operator<=(const simd& lhs, const simd& rhs) noexcept;
friend mask_type operator>(const simd& lhs, const simd& rhs) noexcept;
friend mask_type operator<(const simd& lhs, const simd& rhs) noexcept;
```

## reduction
```
template<class T, class Abi, class BinaryOperation = plus<>>
T reduce(const simd<T, Abi>& x, BinaryOperation binary_op = {});
```
xの全要素に対してbinary_opを適用した値
```
GENERALIZED_SUM(binary_op, x.data[i], ...) for all i in the range of [0, size())
```
を返す

* 初期値`typename V::value_type identity_element`を与えるバージョンもある

```
template<class T, class Abi> T hmin(const simd<T, Abi>& x) noexcept;
```
すべての`i`に対して`x[j] <= x[i]`となる`x[j]`を返す

```
template<class T, class Abi> T hmax(const simd<T, Abi>& x) noexcept;
```
すべての`i`に対して`x[j] >= x[i]`となる`x[j]`を返す

## Cast
```
template<class T, class U, class Abi> see below simd_cast(const simd<U, Abi>& x) noexcept;
```
`simd<U, Abi>`を`simd<T, Abi>`に変換する

simdを分割し直すsplitなどもある

## algorithm
min, max, minmax, clamp(範囲制限)などがある

## mathライブラリ
* `<cmath>`相当の操作がoverloadされる
* `<cmath>`のdomain, pole, range errorなどが起きたときは未定義

## simd_mask
通常の操作の他にreduction系の操作もある
* all_of, any_of, none_of, some_of, popcount, find_first_set, find_last_setなど
* popcountはtrueの個数を返す
* find_first_setはany_of(k)がtrueのときにk[i]がtrueとなる最初のiを返す
* find_last_setはany_of(k)がtrueのときにk[i]がtrueとなる最後のiを返す


## [Technical Specification for C++ Extensions for Parallelism Version 2](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4725.html)
改定案

*注意* : このドキュメントは全く整理されていないメモ書きである。訳の間違いや不足は多分にあると思われる。

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
[P0155R0 | Task Block R5](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0155r0.pdf)
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

### `define_task_block`

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
