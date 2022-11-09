# [CKKS](https://blog.openmined.org/ckks-explained-part-1-simple-encoding-and-decoding/)

## [CKKS (Cheon-Kim-Kim-Song)](https://eprint.iacr.org/2016/421)

全体の流れ
- $m \in  C^{N/2}$ : 平文空間
- $p(X) \in R := Z[X]/(X^N+1)$ : $m$のエンコード
- $c=(c_0(X), c_1(X)) \in (Z_q[X]/(X^N+1))^2$ : 暗号化
- $c'=f(c) \in R^2$ : 評価
- $p'=f(p) \in R$ : 復号
- $m'=f(m) \in C^{N/2}$ : デコード

## 準備
- $N$は2ベキ
- $M=2N$
- $\Phi_M(X)=X^N+1$ : 円分体多項式
  - e.g. $\Phi_8(X)=X^4+1$
- $\xi_M = e^{2 i \pi / M}$ : 1の$M$乗根($X^N+1=\prod_{i=1}^N (X-\xi^{2i-1})$)
- $\xi$ : 1$の原始$M$乗根($\xi^N+1=0$, $\xi^M=1$)
- $\sigma  : C[X]/(X^N+1) \rightarrow C^n$ : 正準埋め込み
  - $m(X) \in C[X]/(X^N+1)$に対して$\sigma(m) := \sigma(m(X))=(m(\xi), m(\xi^3), \dots, m(\xi^{2N-1}))\in C^N$
  - $\sigma(X^N+1)=((\xi^{2i-1})^N+1)=((\xi^N)^{2i-1}+1)=((-1)^{2i-1}+1)=(0)$なのでwell-defined.

これは全単射
- 逆写像
  - $z=(z_1, \dots, z_N) \in C^N$, $m(X)=\sum_{j=0}^{N-1} m_j X^j \in C[X]/(X^N+1)$について$\sigma(m(X))=z$とする
  - $\sum_{j=0}^{N-1}m_j(\xi^{2i-1})^j = z_i$ for $i=1, \dots, N$.
  - $A=((\xi^{2i-1})^j)$とすると$A$はVandermonde行列で$A m= z$より$m = A^{-1} z$

性質
- 和 : $m(X)$, $m'(X)$に対して$\sigma(m+m')=\sigma(m)+\sigma(m')$は自明.
- 積 : $m(X)m'(X)=(\sum_i m_i X^i)(\sum_j m'_j X^j)=\sum_i (\sum_{j=0}^i m_j m'_{i-j})X^i$.
- $\sigma(m)\sigma(m')=(m(\xi^{2i-1})m'(\xi^{2i-1}))$

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
- $RS_{i \rightarrow i-1}(c)=round(p_i^{-1} c)(mod q_{i-1})$
