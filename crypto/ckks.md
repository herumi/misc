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
- $ξ = ξ_M = e^{2 i π / M}$ : 1の$M$乗根
  - $ξ^M = 1$
  - $ξ^N = -1$
  - $N$個の$ξ, ξ^3, ξ^5, ..., ξ^{2N-1}$は全て互いに異なる
  - $X^N + 1 = \prod_{j=1}^N (X - ξ^{2j-1})$ ($N$次多項式は$N$個の解を持つ / 最高次の係数が両辺共に1)
- $C$ : 複素数全体
- $C[X]$ : 複素数係数の多項式全体
  - $C[ X] / (X^N+1)$ : $C[X]$を$X^N+1$で割った余りの多項式全体

### 写像の定義
- $σ: C[ X]→ C^N$を$σ(f) = (f(ξ), f(ξ^3), ..., f(ξ^{2N-1})$とする
  - 多項式$f$, $g$に対して
    - $(f + g)(x) = f(x) + g(x)$, $(fg)(x) = f(x)g(x)$とする
  - $σ(f + g) = σ(f) + σ(g)$
  - $σ(fg) = σ(f)σ(g)$

σは全射($C^N$の全ての値になる多項式がある)
- $f(X) = \sum_{i=0}^{N-1} a_i X^i$とする
- $σ(f) = (\sum_{i=0}^{N-1} a_i (ξ^{2j-1})^i)_{i=0, ..., N-1, j=1, ..., N}$
- $a = (a_0, a_1, ..., a_{N-1})$, $b = σ(f)$, $A=(ξ^{(2j-1)i})_{i=0, ..., N-1, j=1, ..., N}$とすると$b = Aa$
- $A$はVandermonde行列で$ξ^{(2j-1)}$は全て異なるので逆行列がある
- 任意の$b$に対して$b = Aa$となる$a$がある

Ker(σ)
- σの行き先が0になる多項式全体を考える
- f ∈ Ker(σ) ⇔ σ(f) = $(f(ξ), f(ξ^3), ..., f(ξ^{2N-1})=0$ ⇔ $f(ξ^{2j-1})=$ for j = 1, ..., N
  - fは$X - ξ^{2j-1}$ for j = 1, ..., Nで割り切れる ⇔ fは$X^N+1$で割り切れる

σ : $C[ X] / (X^N+1) → C^N$は全単射(同型写像)

対称性
- $ξ^{2j-1}$についてconj($ξ^{2j-1}$)=$ξ^{M-2j+1}$ for j = 1, ..., $N/2$
  - そのような$C^N$の部分空間をHとする
- よって$f$が実数係数多項式なら$σ(f)$は前半分の情報で十分

- $π:  C^N → C^{N/2}$を$π(x_1,...,x_N)=(x_1,...,x_{N/2})$とする
- $π^{-1}: C^{N/2} → H ⊂  C^N$をπ^{-1}(y_1,...,y_{N/2})=(y_1,...,y_{N/2}, conj(y_{N/2}), .., conj(y_1))$とする

### 平文空間とencode, decode
- $z ∈ C^{N/2}$ : 元のデータ
- $π^{-1}(z)∈H$を整数係数に丸める
  - $t = round(π^{-1}(z) Δ)$
  - 丸めるときに誤差を減らすためにΔ倍する
- tを$σ^{-1}$でRにもっていく
- $R = Z[ X]/(X^N+1)$ : 整数係数多項式を$X^N+1$で割った余り全体
- Rが平文空間でzをRに移す操作をencodeという

decode
- $m∈R$に対して$π(σ(m) / Δ)$が元のデータ(の近似値)となる

---

# LWE (Learning with Error)
- $s \in {Z_q}^n$ : 秘密鍵
- $a_i \in {Z_q}^n$ : 一様ランダム
- $e_i \in {Z_q}$ : 小さいノイズ
$(a_i, b_i)=(a_i, a_i \cdot s+e_i)$とランダムな${Z_q}^n \times Z_q$の元が区別つかない
行列$A \in {Z_q}^{n\times n}$を使って$(A, As + e)$とかける

$p=(-As + e, A)$が公開鍵

- $Enc(\mu)$ : $\mu$は平文
  - $c = (c_0,c_1) = Enc(\mu) = (\mu,0)+p=(\mu-As+e,A)$
- $s$で$Dec(c)$を求める
  - $\tilde{\mu}=c_0+c_1 s=\mu-As+e+As=\mu+e \approx \mu$.

# Ring LWE
- ${Z_q}^n$の代わりに$R_q=Z_q[X]/(X^N+1)$を使う
  - 鍵サイズは$O(n)$
  - 多項式の乗算は離散FFTを使うことで$O(n^2)$ではなく$O(n \log(n))$

- $s$ : 秘密鍵
- $p = (b,a)=(-as + e, a) \in {R_q}^2$ : 公開鍵
- $\mu$の暗号化$c=Enc(\mu,p)=(\mu+b,a)$
- $c=(c_0,c_1)$の復号$Dec(c,s)=c_0+c_1s=(\mu+b)+as=\mu+e\approx \mu$

### 準同型性
- $Enc(\mu)=(c_0,c_1)$, $Enc(\mu')=(c'_0,c'_1)$
加算
- $cadd(c,c')=c+c'=(c_0+c'_0,c_1+c'_1)$. $Dec(c+c')=(c_0+c'_0)+(c_1+c'_1)s=\mu+\mu'+(e+e') \approx \mu+\mu'$

### 乗算
- $cmul(c,c')=(d_0,d_1,d_2)=(c_0 c'_0, c_0 c'_1 + c'_0 c_1, c_1 c'_1)$
- $decmul(c,s)=d_0+d_1 s + d_2 s^2$
  - $decmul(c,s)=(c_0 c'_0)+(c_0 c'_1 + c'_0 c_1)s + (c_1 c'_1)s^2=(c_0+c_1s)(c'_0+c'_1s)=(\mu+e)(\mu'+e')=\mu \mu' + \mu e' + \mu' e + e + e' \approx \mu \mu'$

### 再線型化 (relinearization)
- $q$を大きな整数とする。
- 評価鍵 : $e_{vk}=(-a_0s+e_0+p s^2, a_0) mod (pq)$
- $Relin((d_0,d_1,d_2),e_{vk})=(d'_0,d'_1)=(d_0,d_1)+round((1/p)d_2 e_{vk})$
- $Dec((d'_0,d'_1),s)=d'_0+d'_1 s=d_0 + d_1 s + round((1/p)d_2(-a_0 s + e_0 + p s^2 + a_0 s))=(d_0+d_1 s + d_2 s^2)+round(e_0 d_2 / p) \approx d_0 + d_1 s + d_2 s^2 \approx \mu \mu'$

### rescaling
[Microsoft Private AI Bootcamp](https://www.youtube.com/watch?v=SEBdYXxijSo)

- 精度を保つために$z$に対してスケール$\Delta$を掛けておく。$z \Delta$
- 2個の暗号文$c$, $c'$をかけると結果は$ z z' \Delta^2$
- このスケールを元に戻す必要がある
- $L$を乗算回数, $q=\Delta^L q_0 (q_0 \ge \Delta)$とする
  - たとえば10進数部分に30ビット,  整数が10ビットの精度とすると$\Delta=2^{30}$, $q_0=2^{40}$
- $RS(c)=round((q_{l-1}/q_l c)(mod q_{l-1})=round(\Delta^{-1} c)(mod q_{l-1})$
- $L=10$なら$q_L=2^{340}$
  - 大きすぎる

CRT (Chinese remainder theorem)を使う

- $p_1, \dots, p_L$, $q_0$ : 互いに異なり$p_i \approx \Delta$, $q_0 \ge \Delta$なものをとる。
- $q_L=\prod_{i=1}^L p_i q_0$
- $RS_{i → i-1}(c)=round(p_i^{-1} c)(mod q_{i-1})$

[CKKS](https://blog.openmined.org/ckks-explained-part-1-simple-encoding-and-decoding/)
