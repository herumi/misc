# volatileとコンパイラフェンスとメモリフェンス

## なにもつけない
```
void f(int *a)
{
    *a = 1;
    *a = 2;
}
```
コンパイラは最適化して*a = 2;だけにしてもよい。

```
//g++ -Ofast -S -o -
f:
    movl    $2, (%rdi)
    ret
```

## volatile
```
void f(volatile int *a)
{
    *a = 1;
    *a = 2;
}
```
それぞれ書き込み命令を生成する。

```
f:
    movl    $1, (%rdi)
    movl    $2, (%rdi)
    ret
```

しかしvolatileとそうで無い変数の順序は入れ代えてもよい。
```
void f(int *a, volatile int *b)
{
    *a = 1;
    *b = 2;
    *a = 3;
}
```
は
```
f:
    movl    $2, (%rsi)
    movl    $3, (%rdi)
    ret
```
`*a = 1`と`*b = 2`が入れ代わり、`*a = 1`と`*a = 3`が結合した。

## コンパイラフェンス(コンパイラのメモリバリア)
コンパイル時に命令を入れ換えないようにする。

```
void f(int *a)
{
    *a = 1;
#ifdef __GNUC__
    asm volatile("" ::: "memory");
#else
    // for Visual Studio(#include <intrin.h>)
    _ReadWriteBarrier();
#endif
    *a = 2;
}
```
は
```
f:
    movl    $1, (%rdi)
    movl    $2, (%rdi)
    ret
```

## CPUのメモリバリア

CPUは必ずしもアセンブリコードで書かれた順序でプログラムを実行するわけではない。詳細は次章。

# x86/x64におけるメモリオーダーの話

## 概要
ここではアセンブリコードによって書かれたコードがIntel CPU上ではどういう動きをすることが保証されているかについて説明する。
C++のメモリモデルの話ではない。

## 単純な読み書き
アライメントが揃っているときの読み書きは正しく読める。

### アライメント
* 1byteのデータはつねにアライメントされている。
* 2byteのデータはアドレスが2の倍数のときにアライメントされている。
* 4byteのデータはアドレスが4の倍数のときにアライメントされている。
* 8byteのデータはアドレスが8の倍数のときにアライメントされている。

例 x = 0の状態で

cpu1      |   cpu2
----------|---------
mov [x], 1|mov r, [x]

という命令が実行されると考える。ここで[x]はメモリへのアクセス、rはレジスタを表す。
ここでこのときrは0か1のどちらかであることが保証される。

注意
* アライメントが揃っていないときはその限りではない。
* 一般のプロセッサではcpu1でメモリ書き込み中に、別のcpuでそのメモリを読んだとき0でも1でもない値になることはある。

## read同士の順序, write同士の順序はそれぞれ保証される
さらにあるプロセッサの書き込みは他のすべてのプロセッサで同じ順序に見える。

例 x = y = 0の状態で

cpu1      |   cpu2
----------|---------
mov [x], 1|mov r1, [y]
mov [y], 1|mov r2, [x]

* 実行順序のパターン1

```
1. mov [x], 1
2. mov [y], 1
3.            mov r1, [y]
4.            mov r2, [x]
```

このときは(r1, r2) = (1, 1)

* 実行順序のパターン2

```
1. mov [x], 1
2.              mov r1, [y]
3. mov [y], 1
4.              mov r2, [x]
```

このときは(r1, r2) = (0, 1)

* 実行順序のパターン3

```
1.              mov r1, [y]
2.              mov r2, [x]
3. mov [x], 1
4. mov [y], 1
```

このときは(r1, r2) = (0, 0)

cpu1とcpu2のそれぞれの実行タイミングがどうであってもcpu2で観測されるのは(x, y) = (0, 0), (0, 1), (1, 1)のいずれかである。
(r1, r2) = (1, 0)になることはない。

注意
* 一般のプロセッサでは何もしないとこれらの順序が保証されず(1, 0)になることがある。

# readの後のwriteの順序は保証される

例 x = y = 0の状態で

cpu1       |   cpu2
-----------|---------
mov r1, [x]|mov r2, [y]
mov [y], 1 |mov [x], 1

* 実行順序のパターン1

```
1. mov r1, [x]
2. mov [y], 1
3.              mov r2, [y]
4.              mov [x], 1
```

このときは(r1, r2) = (0, 1)

* 実行順序のパターン2

```
1. mov r1, [x]
2.              mov r2, [y]
3. mov [y], 1
4.              mov [x], 1
```

このときは(r1, r2) = (0, 0)

* 実行順序のパターン2

```
1.              mov r2, [y]
2.              mov [x], 1
3. mov r1, [x]
4. mov [y], 1
```

このときは(r1, r2) = (1, 0)

よって(r1, r2)は(0, 0), (0, 1), (1,0)のいずれかであり(r1, r2) = (1, 1)になることはない。
ここまではx86/x64で特に直感に反しない挙動のはなし。次にそうではない例を示す。

注意
* 一般のプロセッサでは何もしないとこれらの順序が保証されず(1, 1)になることがある。

## 異なるメモリへに対するwriteの後のreadの順序は保証されない

例 x = y = 0の状態で

cpu1       |   cpu2
-----------|---------
mov [x], 1 |mov [y], 1
mov r1, [y]|mov r2, [x]

もし順序が保証されていれば(r1, r2)は(0, 1), (1, 1), (1, 0)のいずれかである。
しかし、cpu1においてmov r1, [y]がmov [x], 1よりも先に実行されてしまうことがある。

つまり

```
1. mov r1, [y]
2.                mov [y], 1
3.                mov r2, [x]
4. mov [x], 1
```

と実行されて(r1, r2) = (0, 0)となってしまうことがある。

これを防ぐにはwriteとreadの間にmfence命令をいれるかlockつきのmovにする必要がある。

注意
* デッカーのアルゴリズムは自分のところのフラグをにして相手のフラグが0になるまで待つというコード。
つまり本質は上記のような[x] = 1のあと相手のメモリ[y]を見てwhileする状況。ここでreadとwriteが入れ代わってしまうと
排他すべきループに同時に突入してしまうことがある((0, 0)になってしまうということ)。
* x86においてはデッカーのアルゴリズムでwhileの前にmfenceをいれるのが本質なのでx, yをseq_cstにするとそれ以外の部分で不要なlock命令が入ってしまう。
* release/acquireではそれまでに書いたメモリが全て反映される(release)は保証しているが、それに続くreadがreleaseの境界を下から超えて上に動いてしまうことを除外するものではない。

## x86/x64における一般的なメモリオーダーのルール

### 一つのコアにおいて
* readに続くread(これをR→Rと書くことにする)の順序は変わらない
* readに続くwriteの(R→W)の順序は変わらない
* 一部の命令を除いてW→Wの順序は変わらない
* 同じ場所へのW→Rの順序は変わらない
* 異なる場所へのW→Rの順序は変わるかもしれない
* W→sfence→Wの順序は変わらない
* R→lfence→R, Wの順序は変わらない
* R, W→mfence→R, Wの順序は変わらない

### 複数コアにおいて
* 各プロセッサは1個のプロセッサの順序の原理に従う
* あるプロセッサの書き込みは他の全てのプロセッサにとって同じ順序で見える
* あるプロセッサの書き込みと別のプロセッサの書き込みの順序は保存されない

### lock命令について
lockプレフィクス(F0h)は続く命令のメモリアクセスを排他的にする。

lock可能な命令
```
add, adc, and, btc, btr, bts, cmpxchg, cmpxch8b,
cmpxchg16b, dec, inc, neg, not, or, sbb, sub, xor, xadd, xchg
```
例 `lock add [x], 1`
`std::atomic<int> x; x++;`相当の実装。

ただしxchgのみlockプレフィクスがなくても排他的になる。

* lock命令と他のRWの順序は変わらない

例 x = y = 0, r1 = r3 = 1の状態で

cpu1        |   cpu2
------------|---------
xchg [x], r1|   xchg [y], r3
mov r2, [y] |   mov r4, [x]

のときにr2 = 0かつr4 = 0になることはない。

8.2.3.9 Loads and Stores Are Not Reordered with Locked Instructions参照

### pause命令
ビジーループにおいてpuase命令をはさむとCPUに「今ビジーループをしている」と知らせることでパフォーマンスをあげたり、消費電力を減らしたりできる。
```
.lp:
    cmp [flag], 0
    je  .lock
    pause
    jnz .lp:
.lock:
```

## 参考文献

* Intel 64 and IA-32 Architectures Software Developer's Manual Volum3 3の
8.2.2  Memory Ordering in P6 and More Recent Processor Families

## relaxed, seq_cst, acquire/release, consume
ここらへんから後は気にしなくてよい。

通常C++ではデフォルトのmemory_orderを使うべき。加えて普通使うx86/x64ではそれで問題ない。
下手にやってもバグになる可能性が高い。

### releaxedで読み書きすると

memory_order_relaxed ; 異なるメモリに関するhappned-beforeに関して何も制約が無い

```
x = y = false
x.store(true, relaxed)   | y.load(relaxed)ならx.load(relaxed)とは限らない
y.store(true, relaxed)   |
```

memory_order_seq_cst ; sequentially consistent
どのスレッドでも同じ順序でメモリが見える。
スレッド間で異なるメモリに対するhappned-beforeの推移律が成り立つ

### seq_cstで読み書きすると

x = y = false

```
x = true | y = true
         |xかつ!yだった | => yならxである
```


### acquire/release

storeにrelease, 読み込みにacqure
releaseで書き込んだメモリに対するacquireの読み込みは順序が成り立つ

x = y = false
x.store(true, relaxed)    | y.load(acuire)ならx.load(relaxed)は成り立つ
y.store(true, relaese)    |

```
x = y = false
x.store(true, release) | while (!x.load(acquire));  |
                       | y.store(true, release)     |  while (!y.load(acquire));
                                                    |  ...
```

### memory_order_acq_rel

この真ん中の部分をacq_relを使うと最適化できる。

```
atomic<int> x = 0;
x.compare_exchange_strong(expected, desired);
x != expectedならexpected = xとしてreturn false;
x == expectedならx = desired としてreturn true;
```

```
x.compare_exchange_weak(expected, desired);
x != expectedならexpected = xとしてreturn false;
x == expectedならx = desired としてreturn true;
                                   return false;の可能性もある
```

```
x.store(1, release)    | int expected = 1;
                       | while (!x.compare_exchange_strong(expected, 2, acq_rel) { |
                       |     expected = 1;                                         |
                       | }                                                         | while (x.load(acquire) < 2);
```

### memory_order_consume

C++17から一時的に非推奨 [P0371R1: Temporarily discourage memory_order_consume](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0371r1.html)

carries a dependency toはis sequenced beforeのsubset。
同じメモリに対して依存するもののみ依存関係を保証する。
acquireしなくてもよいときにその最適化を行う。

```
struct X {
    int a;
};
atomic<X*> p = nullptr;
X *x = new X();
x->a = 3;
```

```
p.store(x, release); | while (!p.load(consume));
                     | p->a == 3;
```
