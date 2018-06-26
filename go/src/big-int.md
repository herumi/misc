# Package big.Int

# 概要

[math/big](https://golang.org/pkg/math/big)
多倍長整数演算可能なパッケージ

この分野で有名な多倍長演算ライブラリ[GMP](https://gmplib.org/)のラッパーだろうと思ったら自前だった。

[src/math/big](https://golang.org/src/math/big)
にはx86, x64, arm, arm64, mips, ppcなどのasmファイルがある。

Goのアセンブラは表記も独自。

https://golang.org/doc/asm

元はPlan 9というOSらしい。
そういえば初めてGoをちょろっと触ったとき(2009年頃)、
コンパイラが6gとか8gとか変な名前だなあと思ったのだった。

CPU毎のアセンブラ部分は最小限にしたい。

[arith_amd64.s](https://golang.org/src/math/big/arith_amd64.s)

* mulWWは多分64bit x 64bit→128bit乗算
* divWWは128bit / 64bit除算と余り
* addVV, subVVが同じ長さの多倍長整数同士の加算と減算
    * WはwordでVはvectorだろう
* addVW, subVWが異なる長さの多倍長整数同士の加算と減算
* shlVU, shrVUなどのシフト系
* mulADDVWWという積和
* divWVWという除算がある

https://golang.org/src/math/big/arith.go
はアセンブリ版がない環境用。

これらのプリミティブな関数を使って上位クラスを作っていく。

# [自然数クラス](https://golang.org/src/math/big/nat.go)

```
type nat []Word
```

natはWordのスライス

```
17:
// An unsigned integer x of the form
//
//   x = x[n-1]*_B^(n-1) + x[n-2]*_B^(n-2) + ... + x[1]*_B + x[0]
```

データはlittle endian形式で保持する。
こういうのをちゃんと書かないといけないのだな。 > 自分

```
24:
// A number is normalized if the slice contains no leading 0 digits.
// During arithmetic operations, denormalized values may occur but are
// always normalized before returning the final result. The normalized
// representation of 0 is the empty or nil slice (length = 0).
```

たとえば`x[] = { 2, 1, 3, 0, 0 }`みたいに右側に0入ってることはない(normalizedされる)
0のnormalized表現はlength = 0である

こういうのもちゃんと書く。 > 自分(mclでは0は長さ1で中に0を入れてる)

```
func (z nat) setUint64(x uint64) nat {
	// single-word value
	if w := Word(x); uint64(w) == x {
		return z.setWord(w)
	}
```

uint64 != Wordな環境かをチェックしてる。Wordにキャストしてuint64にもどして一致すればWordで扱える。
uint32環境で常に2-wordにするより効率がよい可能性が高い。これはうまいな。

* func (z nat) add(x, y nat) nat
* func (z nat) sub(x, y nat) nat
* func (x nat) cmp(y nat) (r int)
* func basicMul(z, x, y nat)

などの標準的な実装

## Montgomery乗算

```
// See Gueron, "Efficient Software Implementations of Modular Exponentiation".
// https://eprint.iacr.org/2011/239.pdf
func (z nat) montgomery(x, y, m nat, k Word, n int) nat
```

あとで論文見てみよう。carryをうまく使えば引き算の前の確認が不要?


```
// Operands that are shorter than karatsubaThreshold are multiplied using
// "grade school" multiplication; for longer operands the Karatsuba algorithm
// is used.
var karatsubaThreshold = 40 // computed by calibrate_test.go
```

Karatsuba法を使うかどうかの閾値
calibrate_test.goで決めたそうな。

## func karatsuba(z, x, y nat)

Karatsuba法を使うがサイズを確認しながら再帰的に計算する

```
// alias reports whether x and y share the same base array.
func alias(x, y nat) bool {
	return cap(x) > 0 && cap(y) > 0 && &x[0:cap(x)][cap(x)-1] == &y[0:cap(y)][cap(y)-1]
}
```

work領域を常にとるのは勿体ない。
かと言ってある領域を使い回そうとするとたとえばz.Add(z, z)のときにzを破壊しながら計算してしまうかもしれない。
それを避けるために領域がかぶってるかを確認する。

## 除算divLarge

```
// q = (uIn-r)/v, with 0 <= r < y
// Uses z as storage for q, and u as storage for r if possible.
// See Knuth, Volume 2, section 4.3.1, Algorithm D.
// Preconditions:
//    len(v) >= 2
//    len(uIn) >= len(v)
```

```
// determine if z can be reused
// TODO(gri) should find a better solution - this if statement
//           is very costly (see e.g. time pidigits -s -n 10000)
```

なんか時間がかかってるから速くすべきなんだそうな。

## setBit, and, andNotなど

素直に実装

## expNN, expNNWindowed
`x^y mod m`の計算には4-bit Window法を使う

```
// Unrolled loop for significant performance
// gain. Use go test -bench=".*" in crypto/rsa
// to check performance before making changes.
zz = zz.sqr(z)
zz, z = z, zz
zz, r = zz.div(r, z, m)
z, r = r, z

zz = zz.sqr(z)
zz, z = z, zz
zz, r = zz.div(r, z, m)
z, r = r, z

zz = zz.sqr(z)
zz, z = z, zz
zz, r = zz.div(r, z, m)
z, r = r, z

zz = zz.sqr(z)
zz, z = z, zz
zz, r = zz.div(r, z, m)
z, r = r, z
```

こんな高級言語でもloop unrollが使われることがあるのだなあ。

# [math/big: Exp(x, x, x) returns x sometimes (instead of 0)](https://github.com/golang/go/issues/13907)

```
// One last reduction, just in case.
// See golang.org/issue/13907.
if zz.cmp(m) >= 0 {
	// Common case is m has high bit set; in that case,
	// since zz is the same length as m, there can be just
	// one multiple of m to remove. Just subtract.
```

Montgomery乗算の最後は引き算するかはいつも悩む。

## `func (z nat) bytes(buf []byte) (i int)`
byteへの変換`_S`はどこで定義されてるのだろう。グローバル?
