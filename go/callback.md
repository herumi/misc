# cgoからGoの関数をcallbackとして呼ぶ方法

## 本家マニュアル
* [Command cgo](https://golang.org/cmd/cgo/)
* [cgo](https://github.com/golang/go/wiki/cgo#function-variables)

## GoからCの関数を呼ぶ

* Goのファイルの先頭のコメントの中にCのコードを書いて直後に`import "C"`をする。
* そこで宣言、定義された関数は`C.関数名`の形で呼べる。

```
package main

/*
int add(int x, int y) { return x + y; }
*/
import "C"
import "fmt"

func main() {
    fmt.Printf("add=%d\n", C.add(2, 3)) // add=5
}
```

注意1. コメントと`import "C"`の間に空行を入れるとエラー。

```
// 動かない
/*
int add(int x, int y) { return x + y; }
*/

import "C"
```

注意2. `import "C"`は他のimport文とくっつけられない。
```
/// 動かない
/*
int add(int x, int y) { return x + y; }
*/
import (
    "C"
    "fmt"
)
```

### intとC.intは異なるのでcastが必要。
```
x := 3
C.add(x, 4) // err
C.add(C.int(x), 4) // ok
```

### リンクが必要な関数は`#cgo LDFLAGS: -lm`で呼ぶ。
```
package main
/*
#include <math.h>
#cgo LDFLAGS: -lm
*/
import "C"
import "fmt"

func main() {
    fmt.Printf("sin=%f\n", C.atan(1.0) * 4) // sin=3.141593
}
```

### CFLAGSでコンパイラに指定したマクロも使える。
```
#cgo CFLAGS: -DAAA=3
...
    fmt.Printf("AAA=%d\n", C.AAA) // AAA=3
```
### 文字列ポインタは直接呼べない。

```
    C.puts("abc") // err
```

### ポインタのキャストはunsafe.Pointer経由で行う。
```
import "unsafe"
s := []byte("abc")
C.puts((*C.char)(unsafe.Pointer(&s[0]))) // "abc"を表示
```
byte列sの先頭ポインタ`&s[0]`の`unsafe.Pointer`をとってCの`char*`に変換

### byte列sに対してCの関数で操作する。
上記のように先頭ポインタ`&s[0]`と`len(s)`をキャストして渡す。
```
/*
void fillSeq(char *p, int n)
{
    for (int i = 0; i < n; i++) p[i] = '0' + i;
}
*/
import "C"
import (
    "fmt"
    "unsafe"
)

func main() {
    s := []byte("abcdefg")
    fmt.Printf("s=%s\n", s) // abcdefgを表示
    C.fillSeq((*C.char)(unsafe.Pointer(&s[0])), C.int(len(s)))
    fmt.Printf("s=%s\n", s) // 0123456を表示
```

### CのcharポインタからGoのsliceを作る。
```
func createSlice(p *C.char, n C.size_t) []byte {
    size := int(n)
    return (*[1<<30]byte)(unsafe.Pointer(p))[:size:size]
}
```
createSliceでCのnバイトの領域を持つポインタpに触れる。

```
/*
#include <stdio.h>
static char s_buf[8];
char *getPtr() { return s_buf; }
size_t getSize() { return sizeof(s_buf); }
void putBuf()
{
    printf("putBuf ");
    for (size_t i = 0; i < sizeof(s_buf); i++) {
        printf("%02x ", (unsigned char)s_buf[i]);
    }
    printf("\n");
}
*/
import "C"
import (
    "unsafe"
)

func createSlice(buf *C.char, n C.size_t) []byte {
    size := int(n)
    return (*[1<<30]byte)(unsafe.Pointer(buf))[:size:size]
}

func main() {
    ptr := C.getPtr()
    n := C.getSize()
    s := createSlice(ptr, n)
    C.putBuf()
    for i := 0; i < len(s); i++ {
        s[i] = byte(i)
    }
    C.putBuf()
}
// 実行
putBuf 00 00 00 00 00 00 00 00
putBuf 00 01 02 03 04 05 06 07
```

## createSliceの意味
```
(*[1<<30]byte)(unsafe.Pointer(p))
```
* `*C.char`なpを固定サイズ(1<<30バイト)のbyte列へのポインタに変換する。
* 1<<30はそのプログラムで扱う十分大きなサイズであればなんでもよい。
* 固定長でないとエラーになる。
* `(*[size]byte)(unsafe.Pointer(buf))`はnon-constant array bound sizeのエラー

## s[:size]とs[:size:size]の違い

* 3個目のパラメータはcapacityを表す。[Full slice expressions](https://golang.org/ref/spec#Slice_expressions)参照。
* capacityを指定しないs[:size]は元のbyte列の先頭から長さsizeのsliceでcapacityは元のbyte列の長さを受け継ぐ。
* この場合は`cap(s) = 1<<30`となってしまう。
    * `s = append(s, 'x')`とするとsizeを超えてメモリに追加してしまう。
* s[:size:size]の場合はcapacityもsizeにする。

```
ss := []byte("abcdefg")
fmt.Printf("ss=%s\n", ss)

a1 := ss[:3]
fmt.Printf("a1=%s %p %d %d\n", a1, a1, len(a1), cap(a1))
a1 = append(a1, 'X')
fmt.Printf("ss=%s\n", ss)
fmt.Printf("a1=%s %p %d %d\n", a1, a1, len(a1), cap(a1))

a2 := ss[:3:3]
fmt.Printf("a2=%s %p %d %d\n", a2, a2, len(a2), cap(a2))
a2 = append(a2, 'Y')
fmt.Printf("ss=%s\n", ss)
fmt.Printf("a2=%s %p %d %d\n", a2, a2, len(a2), cap(a2))
```
出力
```
ss=abcdefg
a1=abc 0xc420098010 3 8
ss=abcXefg                 // append(a1, 'X')がssを上書きしている
a1=abcX 0xc420098010 4 8

a2=abc 0xc420098010 3 3
ss=abcXefg                 // append(a2, 'Y')はssを上書きしない
a2=abcY 0xc420098060 4 8   // appendされたポインタが異なっている
```

[capacity.go](capacity.go)

## Cの関数ポインタの中でGoで設定した関数を呼び出す。

例 : intを受けてintを返す関数の[callback](callback)サンプル。

### C側ではcallbackを登録する関数`setCallbackC`がある。
```
typedef int (*FuncType)(int);
void setCallbackC(FuncType f);
```
引数xに対して登録されたcallback関数を呼び出してその結果を出力する関数
```
void callCallbackC(int x);
```
もある。

### Go側では`run(int) int`のinterfaceとそれを設定する関数`setCallbackGo`がある。
```
type CallbackIF interface {
    run(int) int
}

func setCallbackGo(f CallbackIF)
```
Go側で`setCallbackGo`でfを登録したとき`C.callCallbackC`でそのfが呼ばれるようにしたい。

### Goの関数のラッパーを用意する。
* Goの関数は直接Cの関数ポインタに変換できない。
* exportされたGoの関数をCから直接呼ぶことはできる。
    * がCのライブラリから直接その関数を呼ぶと密結合になるのでそれはしたくない。
* →cgoの中で定義した関数からGoのラッパー関数を呼ぶことになる。

### [callback.go](callback/callback.go)
```
var s_callbackIF CallbackIF

func setCallbackGo(f CallbackIF) {
    s_callbackIF = f
}
```
`setCallbackGo`に渡されたfを`s_callbackIF`に保存する。

```
//export wrapCallbackGo
func wrapCallbackGo(x int) int {
   ret := s_callabckIF.run(x)
}
```
* コメントに`//export 関数名`と書くとその関数はglobalに見える。
* `//`と`export`の間にスペースがあるとエラー。
* `export`と`func`の間に改行があるとエラー。
* 関数名とその下で定義する関数名が異なるとエラー。
* `wrapCallbackGo`の中で保存してある`s_callbackIF`を呼び出す。

### [sub.go](callback/sub.go)
* exportされた`wrapCallbackGo`を呼び出すCの関数`wrapCallbackCgo`を作る。

```
package main
/*
#include "lib.h"
int wrapCallbackGo(int); // exported in callback.go
int wrapCallbackCgo(int x)
{
    printf("  wrapCallbackCgo x=%d\n", x);
    int ret = wrapCallbackGo(x + 1);
    printf("  wrapCallbackCgo ret=%d\n", ret);
    return ret;
}
*/
import "C"
```

先程の`setCallbackGo`で上記`wrapCallbackCgo`をsetCallbackCに渡す。
```
func setCallbackGo(f CallbackIF) {
    s_callbackIF = f
    C.setCallbackC(C.FuncType(unsafe.Pointer(C.wrapCallbackCgo)))
}
```
### Cのcallback関数を呼び出すGoのラッパー関数を用意する。
```
func callCallbackGo(x int) {
    C.callCallbackC(C.int(x))
}
```

### 全体の流れ。

```
type Add struct {
    x int
}

func (self *Add) run(x int) int {
    fmt.Printf("      Add.read (self=%x) x=%d\n", self.x, x)
    return x + 1
}
```

`run(int) int`メソッドを持つAddクラスを作る。

```
func main() {
    a := new(Add)
    setCallbackGo(a)
    callCallbackGo(5)
}
```
`CallbackIF`のポインタを`setCallbackGo`に渡して`callCallbackGo`を呼ぶ。

### 関数の流れ

* callCallbackGo
* -> C.callCallbackC
* -> s_callbackC ; Cの関数ポインタ
* -> C.wrapCallbackCgo ; sub.goで定義
* -> wrapCallbackGo ; callback.goで定義されexportされた
* -> s_callbackIF ; Goのinterfaceポインタ
* -> Add.run ; ユーザ定義の関数

### blsでの実装例

* [bls.go](https://github.com/herumi/bls/blob/master/ffi/go/bls/bls.go#L395-L420)
* [callback.go](https://github.com/herumi/bls/blob/master/ffi/go/bls/callback.go)
