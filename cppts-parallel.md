# 並列(parallel)プログラミング

## 参照

* [parallelism-ts](https://github.com/cplusplus/parallelism-ts)

## 目的

C++標準ライブラリに実際に普及している並列プログラミングを導入する。

## 概要

* 名前空間`std::experimental::parallel::v1`にExecutionPolicyパラメータを持つ関数テンプレートを追加する。
* ヘッダファイルは`meow`。
* 並列アルゴリズムは要素アクセス関数経由で引数にアクセスする。

## 実行ポリシー

例

    std::vector<int> v = ...

    // 普通のソート
    std::sort(v.begin(), v.end());

    // 並列版
    using namespace std::experimental::parallel;

    // シーケンシャルソート(従来のソート)を明示
    sort(seq, v.begin(), v.end());

    // 並列ソートを許可
    sort(par, v.begin(), v.end());

    // ベクトル化と並列ソートを許可
    sort(par_vec, v.begin(), v.end());

    // ポリシーの実行時切り換え
    size_t threshold = ...
    execution_policy exec = (v.size() > therashold) ? para : seq;
    sort(exec, v.begin(), v.end());


## ヘッダ`<experimental/execution_policy>`

    namespace std {
    namespace experimental {
    namespace parallel {
    inline namesapce v1 {

    template<class T> struct is_execution_policy;
    template<class T> constexpr bool is_execution_policy_v
      = is_execution_policy<T>::value;

     class sequential_execution_policy;
     class parallel_execution_policy;
     class parallel_vector_execution_policy;
     class execution_policy;
     }}}}

is_execution_policyはどのポリシーの並列実行を行うかを指定する。

* sequential_execution_policy(seq)
    並列実行してはいけないことを示す
* parallel_execution_policy(par)
    並列実行してよいことを示す
* parallel_vector_execution_policy(par_vec)
    ベクトル化と並列実行してよいことを示す
* execution_policy
    実行時にポリシーを決められる

## 並列実行時の振る舞い

* 実行中に必要なメモリがなければstd::bad_allocを投げる。
* 要素アクセス関数が例外を出したとき
    * 実行ポリシーがparならstd::terminateを呼ぶ。
    * 実行ポリシーがseqかparなら
      exception_listを投げる。
* 並列アルゴリズムがstd::bad_allocを投げて終了したのでなければ、すべての例外は呼出元に伝えられる。
  例外が発生したあと、並列アルゴリズムがそのまま進むかそうでないかは不定。

## exception_list

    class exception_list : public exception {
    public:
        typedef unspecified iterator;
        size_t size() const noexcept;
        iterator begin() const noexcept;
        iterator end() const noexcept;
        const char *what() const noexcept override;
    };

* iteratorはForwardIterator;
* size()はexception_listが持つオブジェクトの個数
* begin(), end()がそのオブジェクトの範囲
* what()は何かNTBS(NULL終端文字列)を返す。

## 並列アルゴリズム

* 並列アルゴリズムは要素アクセス関数経由でオブジェクトにアクセスする。
* seqが指定されたときは呼び出しスレッドの中でシーケンシャルオーダーでアクセスする。
* parが指定されるたときは不定数のスレッドから不定の順序で実行されうる。
    * データレースやデットロックを起こさないようにするのは呼び出し側の責任。
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

* par_vecの場合、parに加えて単一スレッドでも複数関数オブジェクトの呼び出しのインタリーブが発生するかもしれない。
    * mutexを使うとデッドロックの危険性が高い。
* 標準関数は次のときにvectorization-unsafeという。
    * メモリ確保、解放以外の関数で、同期するための関数を呼び出しをすることが示されているとき。

### 例

    int x = 0;
    std::mutex m;
    std::vector<int> a = { 0, 1 };
    for_each(par_vec, a.begin(), a.end(),
       [&](int i) {
           m.lock();
           ++x;
           m.unlock();
       }
    );
    // 恐らくデットロックを起こす。

* par, par_vecはシステムリソースが足りないときシーケンシャルな実行になる。
* par, par_vecはInputIteratorを受けたとしても実際にはRandomAccessIteratorだった場合、operator[]を使ってよい。
  この場合、operator[]がレースフリーであることは呼び出し側の責任。
* 処理系依存の型のポリシーを受けたときは、処理系依存。

# アルゴリズム一覧

第1引数にポリシーが入る。

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
       * op.(g_noncomm_sum(op, a_1, ..., a_k), g_noncomm_sum(op, a_(k+1), .., a_N)
       * ただし 1 <= k < N

## for_each

    template<class ExecutionPolicy, class InputIterator, class Function>
    void for_each(ExecutionPolicy&& exec,
                  InputIterator first, InputIterator last,
                  Function f);


std::for_eachと違い並列実行させるときはFunctionはCopyConstructibleでなければならない。

## for_each_n

サイズ指定のfor_each

    template<class ExecutionPolicy, class InputIterator, class Size, class Function>
    void for_each_n(ExecutionPolicy&& exec,
                  InputIterator first, Size n,
                  Function f);


## reduce

標準のaccumulateと同じ。

    template<class InputIterator, class T, class BinaryOperation>
    T reduce(InputIterator first, InputIterator last, T init, BinaryOperation op);

ただしbinary_opが非結合的や非可換の場合、結果は非決定的。
g_sum(op, init, *first, ..., *(first + (last - first) - 1))を返す。

## inclusive_scanとexclusive_scan

    template<class InputIterator, class OputputIterator, class T, class BinaryOperation>
    OutputIterator
    inclusive_scan(InputIterator first, InputIterator last,
                   OutputIterator result, T init, BinaryOperation op);


    template<class InputIterator, class OputputIterator, class T, class BinaryOperation>
    OutputIterator
    exclusive_scan(InputIterator first, InputIterator last,
                   OutputIterator result, T init, BinaryOperation op);


opは[first, last), [result, result + (last - first))区間の値を変更したり、iteratorを無効にしてはいけない。

opが結合的でなければ結果は非決定的。

### inclusive_scan
区間[result, result + (last - first))内の各iに対して
g_noncomm_sum(op, init, *first, ..., *(first + (i - result))を返す。

### exclusive_scan
区間[result, result + (last - first))内の各iに対して
g_noncomm_sum(op, init, *first, ..., *(first + (i - result) - 1))を返す。


inclusiveとexclusiveの差はi番目の要素に対して*(first + (i - result))を含むか含まないかの違い。
