# 並列(parallel)プログラミング

## 目的

C++標準ライブラリに実際に普及している並列プログラミングを導入する。

## 実行ポリシー

並列アルゴリズムは`ExecutionPolicy`という名前のテンプレート引数を持つ関数テンプレートである。

`execution`ヘッダの`std::execution::seq`, `std::execution::par`, `std::execution::par_unseq`の3種類ある。

* `seq` : 従来通り先頭から順に処理する。
* `par` : 複数スレッドを用いて並列実行される。
* `par_unseq` : `par`かつSIMD的なベクトル実行される。
    * `unseq`のみも欲しいと行ったけどdeniedされた。

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


# アルゴリズム一覧

第1引数にポリシーが入る(古い情報なので要確認)。

    adjacent_difference adjacent_find all_of any_of copy copy_if copy_n count
    count_if equal exclusive_scan fill fill_n find find_end find_first_of find_if
    find_if_not for_each for_each_n generate generate_n includes inclusive_scan
    inner_product inplace_merge is_heap is_heap_until is_partitioned is_sorted
    is_sorted_until lexicographical_compare max_element merge min_element
    minmax_element mismatch move none_of nth_element partial_sort partial_sort_copy
    partition partition_copy reduce remove remove_copy remove_copy_if remove_if
    replace replace_copy replace_copy_if replace_if reverse reverse_copy rotate
    rotate_copy search search_n set_difference set_intersection
    set_symmetric_difference set_union sort stable_partition stable_sort swap_ranges
    transform uninitialized_copy uninitialized_copy_n uninitialized_fill
    uninitialized_fill_n unique unique_copy


## 標準関数にあるけど上記一覧にないもの(抜けがあるかも)

   accumulate binary_search copy_backward equal_range is_permutation
   make_heap next_permutation lower_bound
   partial_sum partition_point pop_heap prev_permutation push_heap
   sort_heap suffle upper_bound

## 標準関数に無くて上記一覧に入っているもの

   for_each_n reduce inclusive_scan exclusive_scan

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
    template<class ExecutionPolicy, class InputIterator, class Function>
    void for_each(ExecutionPolicy&& exec,
                  InputIterator first, InputIterator last,
                  Function f);
```

std::for_eachと違い並列実行させるときはFunctionはCopyConstructibleでなければならない。

## for_each_n

サイズ指定のfor_each
```
    template<class ExecutionPolicy, class InputIterator, class Size, class Function>
    void for_each_n(ExecutionPolicy&& exec,
                  InputIterator first, Size n,
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
