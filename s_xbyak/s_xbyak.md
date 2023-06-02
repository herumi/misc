---
marp: true
---
<!--
headingDivider: 1
-->
<style>section {justify-content: start;}</style>

# Xbyakライクなx64用静的ASM生成ツール s_xbyak

# 背景
<!--
paginate: true
-->

- 暗号ライブラリmclの有限体実装はXbyakを使っていた。
- アセンブリ言語を慣れた言語（C++）で記述できるのはとても便利。
- JITが必要でないところもXbyakを使って書いていた。
- セキュリティ上JITは最小限にしたい。
  - 今まではJITコードをバイナリダンプしてASMファイルに変換していた。
- 最初から静的ASM（普通のASM）を生成したい。
  - 言語はC++よりお手軽なPythonで。→ s_xbyak

# 有限体の演算
- 実装に必要なものはmov/add/mul/などの基本的な命令が殆ど
  - とりあえず必要最小限の機能を実装 [初版](https://github.com/herumi/mcl/blob/318f3904aaa666d174c4d60af22f391335413a5b/src/gen_x86asm.py)は270行ほど
- 試してみると便利なことが分かる
- 機能追加
  - ラベル・ジャンプ
    - ラベルはXbyakのラベル周りの仕様と実装をPythonに移植
  - データ配置
  - GAS/MASM対応
    - 最初はNASMだけのつもりだった
    - gcc *.cpp *.Sで一緒にコンパイルできるのが便利
    - Visual StudioにはMASMがついてくる

# AVXにも対応したい
- [simdgen](https://github.com/herumi/simdgen) : 関数を文字列として入力してJITコード生成
  - 静的ASM版も欲しい
  - [fmath](https://github.com/herumi/fmath)への組み込み
- Xbyakの記法をなるべくs_xbyakに持ってきたい
- AVX-512に真面目に対応する
  - AVX-512はとても大変
  - [x64用主要アセンブラの構文差異クイズ](https://zenn.dev/herumi/articles/s_xbyak-assembler-2)

# AVX-512の記法例

s_xbyak
```python
vaddps(zmm0, zmm1, ptr_b(rax))
```
raxのアドレスのfloatを16個複製してzmm1の要素それぞれに加算してzmm0に格納

MASM : vaddps `zmm0, zmm1, dword bcst [rax]`
NASM : vaddps `zmm0, zmm1, [rax]{1to16}`
GAS : `vaddps (%rax){1to16}, %zmm1, %zmm0`

- vaddpsの`ps`はpacked float(32bit)でzmm(512bit)を使っているから512/32=16個複製なのは分かること。→ 冗長
- Xbyakでは`ptr_b`ですませられるようにしている。

# `ptr_b`の処理
- `vaddps`と`zmm`なら`{1to16}`。`xmm`なら`{1to4}`。
- 命令とレジスタの組み合わせから`{1toX}`を決定しなければならない。
  - 殆どの命令は`ps`や`pd`とレジスタで決まる（最初はこれでやっていた）。
  - そうでないものが出る度に例外パターン追加。
    - 例外パターンが増殖。
    - 全てのパターンをテーブルで持つしかない。
- Intel SDMのPDFからpdftotextで文字列を抽出。
  - 適当に命令っぽいものを取り出して余計なものを除外。
  - 最初は適当なパターンマッチで取り出していたが取りこぼし多し。

# 属性処理

```
vcmppd k4{k3}{sae}, zmm1, zmm2, 5 // Intel XED
```

```python
vcmppd(k4|k3|T_sae, zmm1, zmm2, 5) // Xbyak
```

位置が微妙に違う
```
vcmppd k4{k3}, zmm1, zmm2, 5{sae} // MASM
vcmppd k4{k3}, zmm1, zmm2, {sae}, 5 // NASM
vcmppd $5, {sae}, %zmm2, %zmm1, %k4{%k3} // GAS
```
その他
- 未対応なもの
- エラーになるもの（マイナーなので）

# exp再登場

[gen_fmath.py](https://github.com/herumi/fmath/blob/dev/gen_fmath.py)
アルゴリズムのおさらい

- $e^x = 2^{x log_2(e)}$
- $y=x log_2(e)$ と置いて $y = n + a$ ($n$ は整数, $|a|\le 1/2$)とする。
- $e^x = 2^y = 2^n \times 2^a$
- $2^a$ を多項式近似する。
  - $2^a = 1 + C_1 a + C_2 a^2 + C_3 a^3 + C_4 a^4 + C_5 a^5$
$\,\,\,\,\,\,\,\,=1 + a(C_1 + a(C_2 + a(C_3 + a(C_4 + a C_5))))$

# AVX-512でのfloatから整数への変換
- $y = n + a$ ($n$ は整数, $|a|\le 1/2$)とする。
  - $n = round(y)$
  - `vrndscaleps`(x, y, imm)`
    - $2^{-M}RoundToInt(x*2^M, roundCtl)$
    - ここで $M$ と roundCtlは`imm`から決まる。
    - `imm = 0`なら$M=0$, roundCtl=nearest even integer
- $a = y - n$

```python
# v0 = x
vrndscaleps(v1, v0, 0) # n = round(x)
vsubps(v0, v0, v1) # a = x - n
```

# AVX-512での$x \times 2^n$
従来は
1. floatとしての $n$ をintとしての $n$ にconvert
2. floatのバイナリ表現として指数部に $n$ を挿入(整数の引き算とビットシフト)
3. それをfloatとして扱い、$x$ と乗算

をする必要があった。

- `vscaleps(z, x, y)` : $z = x \times 2^{floor(y)}$
- `vscaleps(z, x, n)` : $z = x \times 2^n$ : 1命令でできる。

# 基本コード
```python
vmulps(v0, v0, self.log2_e)
vrndscaleps(v1, v0, 0) # n = round(x)
vsubps(v0, v0, v1) # a = x - n

vmovaps(v2, self.expCoeff[5])
for i in range(4, -1, -1):
  vfmadd213ps(v2, v0, self.expCoeff[i]) # v2 = v2 * a + C[i]
vscalefps(v0, v2, v1) # v2 * 2^v1
```

# ループアンロール1/5
```python
vmulps(zm0, zm0, self.log2_e)
```
↓
```python
vmulps(zm0, zm0, self.log2_e)
vmulps(zm1, zm1, self.log2_e)
vmulps(zm2, zm2, self.log2_e)
...
```
としたい。
```python
vmulps([zm0, zm1, zm2], [zm0, zm1, zm2], self.log2_e) # 案
```
- 配列ならそれぞれの要素同士
- 単体なら固定

# ループアンロール2/5
メモリアクセス
```python
vmovups([zmm0, zmm1, zmm2], ptr(rax))
```
↓
```python
vmovups(zmm0, ptr(rax+64*0))
vmovups(zmm1, ptr(rax+64*1))
vmovups(zmm2, ptr(rax+64*2))
```
としたい。

# ループアンロール3/5
```python
def Unroll(n, op, *args):
  xs = list(args)
  for i in range(n):
    ys = []
    for e in xs:
      if isinstance(e, list): # 引数が配列ならi番目
        ys.append(e[i])
      elif isinstance(e, Address) and not e.broadcast: # 引数がアドレスでbroadcastでない
        ys.append(e + SIMD_BYTE*i) # SIMDサイズのオフセット
      else:
        ys.append(e) # 単体はそのまま
    op(*ys)
```

# ループアンロール4/5
- Unroll関数を使う
- nを引数にとり「opを引数にとり「*argsをUnrollする関数」を返す」関数
```python
def genUnrollFunc(n):
  def fn(op):
    def gn(*args):
      Unroll(n, op, *args)
    return gn
  return fn
```

# ループアンロール5/5
使い方
```python
un = genUnrollFunc(n)
un(vmulps)(v0, v0, self.log2_e)
un(vrndscaleps)(v1, v0, 0) # n = round(x)
un(vsubps)(v0, v0, v1) # a = x - n

un(vmovaps)(v2, self.expCoeff[5])
for i in range(4, -1, -1):
  un(vfmadd213ps)(v2, v1, self.expCoeff[i])
un(vscalefps)(v0, v2, v0) # v2 * 2^v1
```

# ベンチマーク

-|1|2|3|4|5|6|7|8
-|-|-|-|-|-|-|-|-
unroll|10.2 | 9.0 | 8.0 | 7.6 | 7.1 | 7.2 | 7.0 | 7.2

- unroll=7あたりが一番速い

# 各種命令の速さ

SkyLake
命令|ポート|レイテンシ|スループットの逆数
-|-|-|-
vaddps|p05|4|0.5-1
vmulps|p05|4|0.5-1
vrndscaleps|2p01/05|8|1
vreduceps|p01|4|0.5-1
vscaleps|p01|4|0.5-1
vfma|p05|4|0.5-1

# 速度改善
- `vreduceps(y, x, ctrl)` # $y = x - round(2^M \times x)\times 2^{-M}$
- 先に小数部分を求める命令があった。

```python
vrndscaleps(v1, v0, 0) # n = round(x)
vsubps(v0, v0, v1) # a = x - n
```
↓
```python
un(vreduceps)(v1, v0, 0) # a = x - n
un(vsubps)(v0, v0, v1) # n = x - a = round(x)
```
- round()してから小数部分を求めるのは`vrndscaleps+vsubps`の結果待ち。
- 先に`vreduceps`すると`vsubps`の結果はしばらく後でもいい。

# ベンチマーク

-|1|2|3|4|5|6|7|8
-|-|-|-|-|-|-|-|-
改善前|10.2 | 9.0 | 8.0 | 7.6 | 7.1 | 7.2 | 7.0 | 7.2
改善後| 8.9 | 8.0 | 7.1 | 6.9 | 6.8 | 6.6 | 6.5 | 6.7
