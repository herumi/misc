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

* sequential_execution_policy
    並列実行してはいけないことを示す
* parallel_execution_policy
    並列実行してよいことを示す
* parallel_vector_execution_policy
    ベクトル化と並列実行してよいことを示す
* execution_policy
    実行時にポリシーを決められる

## 並列実行時の振る舞い

* 実行中に必要なメモリがなければstd::bad_allocを投げる。
* 要素アクセス関数が例外を出したとき
    * 実行ポリシーがparallel_vector_execution_policyならstd::terminateを呼ぶ。
    * 実行ポリシーがsequential_execution_policyかparallel_execution_policyなら
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
* sequential_execution_policyが指定されたときは呼び出しスレッドの中でシーケンシャルオーダーでアクセスする。
