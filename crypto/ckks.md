# [CKKS (Cheon-Kim-Kim-Song)](https://eprint.iacr.org/2016/421)

## 概要
- 完全準同型暗号
- LWE仮定
- 平文が整数係数の多項式
- 復号したら誤差を伴う暗号方式
- 乗算するときに評価鍵を用いて暗号文サイズを減らす

## 数学の準備

### 記号の定義

- $M$ : 2のべき乗
- $N = M/2$
- $ξ = ξ_M = e^{2 i π / M}$ : 1の $M$ 乗根
  - $ξ^M = 1$
  - $ξ^N = -1$
  - $N$ 個の $ξ, ξ^3, ξ^5, ..., ξ^{2N-1}$ は全て互いに異なる
  - $X^N + 1 = \prod_{j=1}^N (X - ξ^{2j-1})$  ( $N$ 次多項式は $N$ 個の解を持つ / 最高次の係数が両辺共に1)
- $C$ : 複素数全体
- $C[X]$ : 複素数係数の多項式全体
  - $C[ X] / (X^N+1)$ : $C[X]$を $X^N+1$ で割った余りの多項式全体

### 写像の定義
- $σ: C[ X]→ C^N$ を $σ(f) = (f(ξ), f(ξ^3), ..., f(ξ^{2N-1})$ とする
  - 多項式 $f$, $g$ に対して
    - $(f + g)(x) = f(x) + g(x)$, $(fg)(x) = f(x)g(x)$ とする
  - $σ(f + g) = σ(f) + σ(g)$
  - $σ(fg) = σ(f)σ(g)$

σは全射( $C^N$ の全ての値になる多項式がある)
- $f(X) = \sum_{i=0}^{N-1} a_i X^i$ とする
- $σ(f) = (\sum_{i=0}^{N-1} a_i (ξ^{2j-1})^i)_{i=0, ..., N-1, j=1, ..., N}$
- $a = (a_0, a_1, ..., a_{N-1})$, $b = σ(f)$, $A=(ξ^{(2j-1)i})_{i=0, ..., N-1, j=1, ..., N}$ とすると $b = Aa$
- $A$ はVandermonde行列で $ξ^{(2j-1)}$ は全て異なるので逆行列がある
- 任意の $b$ に対して $b = Aa$ となる $a$ がある

Ker(σ)
- σの行き先が0になる多項式全体を考える
- f ∈ Ker(σ) ⇔ σ(f) = $(f(ξ), f(ξ^3), ..., f(ξ^{2N-1})=0$ ⇔ $f(ξ^{2j-1})=0$ for j = 1, ..., N
  - fは $X - ξ^{2j-1}$ for j = 1, ..., Nで割り切れる ⇔ fは $X^N+1$ で割り切れる

σ : $C[ X] / (X^N+1) → C^N$は全単射(同型写像)

対称性
- $ξ^{2j-1}$についてconj( $ξ^{2j-1}$ )= $ξ^{M-2j+1}$ for j = 1, ..., $N/2$
  - そのような $C^N$ の部分空間をHとする
- よって $f$ が実数係数多項式なら $σ(f)$ は前半分の情報で十分

- $π:  C^N → C^{N/2}$ を $π(x_1,...,x_N)=(x_1,...,x_{N/2})$ とする
- $π^{-1}: C^{N/2} → H ⊂  C^N$ を $π^{-1}(y_1,...,y_{N/2})=(y_1,...,y_{N/2}, conj(y_{N/2}), .., conj(y_1))$ とする

### 平文空間とencode, decode
- $z ∈ C^{N/2}$ : 元のデータ
- $π^{-1}(z)∈H$ を整数係数に丸める
  - $t = round(π^{-1}(z) Δ)$
  - 丸めるときに誤差を減らすためにΔ倍する
- tを $σ^{-1}$ で R にもっていく
- $R = Z[ X]/(X^N+1)$ : 整数係数多項式を $X^N+1$ で割った余り全体
- Rが平文空間でzをRに移す操作をencodeという

decode
- $m∈R$ に対して $π(σ(m) / Δ)$ が元のデータ(の近似値)となる

### encode/decodeによる誤差の例
- 行列 $A$ の逆行列の一列目は $(2/M, ..., 2/M)$
- よって $z = (-M/8, 0, 0, ..., 0)$ に対して $σ^{-1}(π^{-1}(z))$ の定数項は $-1/2$ に近い
- これが丸めによって0になるとencode→decodeの処理により $z$ の最初の項が0になりえる
- M = 2048 なら256→0になる

## LWE (Learning with Error)
- $Z_q = \{0, 1, ... q-1\}$ : qで割った余りの整数の集合
- $s \in {Z_q}^n$ : 秘密鍵
- $a_i \in {Z_q}^n$ : 一様ランダム
- $e_i \in {Z_q}$ : 小さいノイズ

### LWE仮定

- $(a_i, b_i)=(a_i, a_i \cdot s+e_i)$ とランダムな ${Z_q}^n \times Z_q$ の元が区別つかない
- 行列 $A \in {Z_q}^{n\times n}$ とベクトル $e \in {Z_q}^n$ を使って $(A, As + e)$ とするとこれから $s$ を求められない

### Ring LWE
- ${Z_q}^N$ の代わりに $R_q=Z_q[X]/(X^N+1)$ を使う
  - 鍵サイズは $O(N)$
  - 離散FFTを使うことで多項式の乗算コストは $O(N^2)$ から $O(N \log(N))$ に減る

### 暗号化と復号のキーアイデア
- 鍵生成
  - $sk = s$ : 秘密鍵(N次元の{0,±1}の乱数)
  - a を一様ランダム, e を小さいノイズとして $pk = (b,a)=(-as + e, a) \in {R_q}^2$ : 公開鍵
- $m$ の暗号化
  - $c=Enc(pk, m)=v pk + (m + e_1, e_2) = (m + v(-as+e) + e_1, va + e_2)$, $v$ は{0,±1}の乱数ベクトル, $e_i$ は小さいノイズ
- $c=(c_0,c_1)$ の復号
  - $Dec(sk, c)=c_0+c_1s=(m + v(-as + e) + e_1) + (va + e_2)s = m + ve + e_1 + e_2s = m + \tilde{e}~ \approx m$

## 準同型性
- $Enc(m)=(c_0,c_1)$, $Enc(m')=(c'_0,c'_1)$

### 加算
- $add(c, c') = c+c'=(c_0+c'_0,c_1+c'_1)$
- $Dec(c+c')=(c_0+c'_0)+(c_1+c'_1)s= (m + \tilde{e}) + (m' + \tilde{e}') \approx \mu+\mu' = Dec(c) + Dec(c')$

### 乗算は?
- $(c_0 + c_1 s)(c'_0 + c'_1 s) = (c_0 c'_0) + (c_0 c'_1 + c_1 c'_0)s + (c_1 c'_1)s^2 = d_0 + d_1 s + d_2 s^2$
- 左辺(LHS)は $=(m + \tilde{e})(m' + \tilde{e}') = mm' + (m \tilde{e}' + m' \tilde{e} + \tilde{e}\tilde{e}')$
  - もし2項目が $mm'$ より小さければ $LHS \approx mm'$ と期待するものになる
- 右辺(RHS)は $mul(c, c') = (d_0, d_1, d_2)$, $Dec(sk, mul(c, c')) = d_0 + d_1 s + d_2 s^2$ とすればOK
  - ただし乗算した暗号文のサイズが大きくなる

### 再線型化 (relinearization)
- 乗算暗号文を元のサイズに小さくする
- そのために評価鍵を導入する(乗算に必要な公開鍵の一種)
- 新たなパラメータ P を比較的大きな整数とする
- 評価鍵 : $a' ~ a$ をランダム, $e''$ をノイズとして $ek=(-a' s+e'' +P s^2, a')$ とする
  - ekから s は求まらない
- 暗号文 $(d_0, d_1, d_2)$の代わりに $(d_0,d_1)+round((1/P)d_2 ek)$ を $mul(c, c')$ とする
- $Dec(sk, (d'_0, d'_1))=d'_0+d'_1 s=d_0 + d_1 s + (1/P) d_2 ((-a' s + e'' + P s^2)+a's)$
  - $=(d_0 + d_1 s + d_2 s^2) + d_2/P e''$
  - 第2項は小さいノイズ
  - よって $Dec(sk, mul(c, c')) \approx mm'$

### 再スケーリング (rescaling)
- 精度を保つために元のデータ $z$ に対してスケール Δ を掛けている
- 2個の暗号文 $c$, $c'$ をかけると結果は $z z' Δ^2$ となる
- このスケールを元に戻す必要がある

### 実際には
- 多項式の乗算は離散FFTを使う
- modの計算はCRT (Chinese remainder theorem)を使う

参考
- [Microsoft Private AI Bootcamp](https://www.youtube.com/watch?v=SEBdYXxijSo)
- [CKKS](https://blog.openmined.org/ckks-explained-part-1-simple-encoding-and-decoding/)
