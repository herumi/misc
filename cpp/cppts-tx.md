# トランザクショナルメモリ

## 参照

* [Experimental C++ standard libraries](http://en.cppreference.com/w/cpp/experimental)
* [ML](https://groups.google.com/a/isocpp.org/forum/#!aboutgroup/tm)
* [N4265:Transactional Memory Support for C++: Wording (revision 3)](https://isocpp.org/files/papers/n4265.html)
* [pdf:https://groups.google.com/a/isocpp.org/group/tm/attach/8b50f7aa6cabf94b/Technical%20Specification%20for%20C++%20Extensions%20for%20Transactional%20Memory,%20Working%20Draft,.pdf?part=0.1]
## 目的

C++コアと標準ライブラリにトランザクショナルメモリ(以下tx)を提供する。

## 概要
### synchronized構文。
```
int f()
{
  static int i = 0;
  synchronized {
    printf("before %d\n", i);
    ++i;
    printf("after %d\n", i);
    return i;
  }
}
```
複数スレッドからのf()の呼び出しはbefore/afterの値は(オーバーフローを除いて)必ず1増えている。


### atomic構文。
```
atomic_noexcept { ... }
atomic_cancel { ... }
atomic_commit { ... }
```

```
int f()
{
  static int i = 0;
  atomic_noexcept {
    ++i;
    return i;
  }
}
```
複数スレッドからのf()の呼び出しもmutexで守られているかのように正しく処理される。

### syncrhonized構文に対する最適化のヒント
`optimize_for_synchronized`属性はその関数がsynchronized構文からの呼び出しで最適化されるべきであることを示す。
```
// translation unit 1
[[optimize_for_synchronized]] int f(int);

void g(int x) {
  synchronized {
    int ret = f(x*x);
  }
}

// translation unit 2
#include <iostream>

extern int verbose;

[[optimize_for_synchronized]] int f(int x)
{
  if (x >= 0)
    return x;
  if (verbose > 1)
    std::cerr << "failure: negative x" << std::endl;
  return -1;
}
```
もしfに`optimize_for_synchronized`がなく、`transaction_safe`宣言されていなければいつもgのsynchronized blockの中で投機実行が中断されてしまう。

### 宣言に追加
```
(cv-qualifier) (ref-qualifier) (tx-qualifier) (exception-specification) (attribute-specifier)
```
* transaction_safe_noinherit
* transaction_safe

transaction_safeで定義された仮想関数のoverrideはtx_safe。

```
struct B {
  virtual void f() transaction_safe;
  virtual ~B() transaction_safe_noinherit;
};

// pre-existing code
struct D1 : B
{
  void f() override { }   // ok
  ~D1() override { }   // ok
};

struct D2 : B
{
  void f() override { std::cout << "D2::f" << std::endl; }
       // error: transaction-safe f has transaction-unsafe definition
  ~D2() override { std::cout << "~D2" << std::endl; }     // ok
};

struct D3 : B
{
  void f() transaction_safe_noinherit override;
         // error: B::f() is transaction_safe
};

int main()
{
  D2 * d2 = new D2;
  B * b2 = d2;
  atomic_commit {
    B b;        // ok
    D1 d1;      // ok
    B& b1 = d1;
    D2 x;       // error: destructor of D2 is not transaction-safe
    b1.f();     // ok, calls D1::f()
    delete b2;  // undefined behavior: calls unsafe destructor of D2
  }
}
```

### tx-safeな関数の定義
式が次のいずれかを含めばtx-unsafe
* volatile glvalueに適用されるlvalue-to-rvalue変換
* volatile glvalueを通してオブジェクトを変更する表現
* volatile属性を持つ一時オブジェクトの生成
* a function call (5.2.2 expr.call) whose postfix-expression is an id-expression that names a non-virtual function that is not transaction-safe
  * ???
* tx-safeでない非仮想関数の暗黙の呼び出し
* tx-safeでない関数の呼び出し

文が次のいずれかであればtx-unsafe
* 式がtx-unsafe
* asm文
* volatileの宣言
* 文がtx-unsafe(再帰的)
```
  extern volatile int * p = 0;
  struct S {
    virtual ~S();
  };

  int f() transaction_safe {
    int x = 0;   // ok: not volatile
    p = &x;      // ok: the pointer is not volatile
    int i = *p;      // error: read through volatile glvalue
    S s;         // error: invocation of unsafe destructor
  }
```
```
  int f(int x) {    // is transaction-safe
    if (x <= 0)
      return 0;
    return x + f(x-1);
  }
```

## 解決されていない問題

* 例外ハンドラがstd::terminateを呼び出したとき。
terminateハンドラはtransaction_safeで宣言されていない。

* static変数の初期化
* 共有メモリにアクセスしないatomic blockはinter-thread-happens-beforeを受けたり引き起こしたりすべきではない。
つまりatomic blockはfenceの代用として使えない。
これにより、もしatomic blockが完全に局所的ならコンパイラの最適化によってハードウェアレベルの同期コストを排除できる。
* 最初sin(x)などのmath.hの関数はtransaction-safeにできると思ったが、それらは現在の丸めモードのような状態を見る必要があるかもしれない。
そうするとtransactionな実行にフィットしない。
* std::functionの使用
* global mutexをとっているかのようなsynchronized block

## 解決された問題
* どういう状況ならラムダ関数はtx-safeに宣言されるのか。
→ 呼ばれている関数がすべてtx-safeならtx-safe。
* atomic blockとtx-safeという用語を使う。
* tx-safeメンバ関数へのポインタからメンバー関数へのポインタの変換を許可する。
* 仮想関数に対してtransaction_safe noinheritを追加する。
* std::exceptionにtransaction_safeなwhat()が必要か。
→ 仮想関数に対して"maybe tx_safe"を追加する。
* 次のクラスを導入する。
```
  namespace std {
    template<class T>
    class tx_exception : public runtime_error {
    public:
       explicit tx_exception(T value) transaction_safe;
       tx_exception(T value, const char* what_arg) transaction_safe;
       tx_exception(T value, const string& what_arg) transaction_safe;
       T get() const transaction_safe;
    };
  }
```
この`what()`はtx_safeで、`T`はmemcpy可能なものである。

# 1.10 マルチスレッドとデータレース
synchronized blockやatomic blockは別のsynchronized blockやatomic blockと実行時のネストはできない。
全ての外側のblockに対して全順序がある。その順序に関してT1かT2の前なら、T1の同期が終わってからT2が始まる。

## 評価の順序
```
   C --- A --- B
      <--T-->
```
atomic block TがAの評価の前に始まり(seq. before)、Aの評価のあとに終わったとする。
Bの評価の前にAがあるなら、Tの終わりはBの前にある。
Cの評価の後にAがあるなら、Tの始まりはCの後にある。

## 変換
* tx-safeな関数のlvalueは関数ポインタのprvalueに変換できる。
* tx-safeな関数へのポインタはメンバ関数ポインタのprvalueに変換できる。

## 継承
syncronized/atomic block内での`transaction_safe_noinherit`がついてない仮想関数の呼び出しは未定義。


## 古いメモ
以下は[N3919:Transactional Memory Support for C++](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3919.pdf)を見たときのメモや感想など。

### synchronizedとatomic blockを排他する必要があるのか。

現状だとユーザにとって排他が必要がないときでもatomic blockがsynchronizedを
知っている必要があります。互いに無関心の方がコストは低いです。

sync-sync, sync-atomicの排他を実現するには実装はたとえばこんな感じでしょうか。

globalなshared mutex(gm)を用意します。
ここでshraed mutexとはwriteLockとreadLockを持ち、
writeLockされたgmに対してwriteLock(), readLock()はブロックする、
readLockされたgmに対してwriteLock()はブロック、readLock()はブロックしない
とします(C++14のshared_mutexならlockとlock_shared)。
```
synchronized {
    writeLock(gm);
    // 本来の処理
}
```
```
atomic block {
    readLock(gm);
    // atomic用の排他制御
    // 本来の処理
}
```
atomic blockではこのlockの後にatomic本来のlock機構が入ることになるでしょう。

Haswellの場合はsynchronized blockではHLEを使ったmutex lock,
atomic blockではRTMを使うことで小さいコストで実装できそうです。
しかし、そうでないCPUではコストが高いと思われます。

### atomic blockの中のsynchornizedを許す必要があるのか。
synchronizedの中でtransaction_unsafeなものを呼んだときどうするのか。
```
// 例
atomic_XXX {
   synchronized {
     transaction_unsafe_func();
   }
}
```
禁止した方が実装コストも仕様も簡単になると思います。

### atomic blockのp.4 closed nesting
closedではなくflattenで十分だと思います。
内側で例外が出たら外に一気に出た方が巻き戻しの再帰が不要で簡単です。
HaswellのTSX(=HLE + RTM)はflatten型なのでclosedにするには巻き戻し操作が
別途必要になります。

### synchronized blockのdomain
p.9にあるような実行時にチェックして巻き戻しという操作は高コストすぎる。

そうではなく外側のsynchronizedで指定されたdomainは内側のsynchronizedで
指定された全てのdomainを指摘したものと同じ挙動をするという仕様ならあり
だと思います。ただしこれは静的解析可能な範囲しかできないです。
たとえば関数ポインタを含むと無理です。

### 10.10
foo()の実装が異なるコンパイル単位で二つあり、片方がtransaction_safe、
もう片方がtransaction_unsafeの場合リンクエラーになるのか、それとも
safeなfoo()をよんでしまうのか(間違えて作った場合に気がつけるのか)。

### 10.13
get(0, 1)をとらない実装はかなりコストが高いのではと思います。
atomic blockの中はatomic操作を含まないというぐらいのきつい制約を
入れてもよいのではないでしょうか。

### atomic_xxxの意図全般
これはトランザクションメモリで実装した場合、エラーになったとき成功するまで
リトライすることを想定していて内部については何も触れてないと思われます。

その様な高度に抽象化されたものではなく、RTMを軽くラップしたもの、

たとえば
```
atomic {
    atomicに実行されるごく簡単な操作
    std::commit(); // commit()を呼ぶとコミットされる。呼ばなければキャンセル
} aborted {
   競合が発生してトランザクションが中止されたときに来る
} committed {
   ランザクションがコミットされたときに来る
}
```
というようなもので、トランザクション失敗時は自分でりトライするか
別の操作をするか選べる方が実用的に思います。

もしここまで抽象化をしたものを入れるのならtransaction memoryだけでなく、
本来ロールバックできる操作をatomic_xxxで扱う方が将来DB系をサポートする
ときが来たときに一貫した操作ができるのではないでしょうか。

### typo

> p.3 4 atomic_noexcept
> ...not allowed; no side effects of the transation can be observed.
>                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
>                 不要? コピペミス?
