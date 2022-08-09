# Montgomery演算

## 記号
- p : prime (>= 3)
- N = 2^L (L = 32 or 64を想定)
(-p)とNは互いに素なのでR(-p)+aN=1となる整数R(0 < R < N)とaがある。

このとき
R=(-p^(-1)) % N
である。

## Th 1. 任意の整数xに対してy := x + p((xR) % N)はNで割り切れる。

pR+1=aNなのでpxR+x=axN。
xR = ((xR) % N) + bNとすると
y = x + p(xR % N) = x + p(xR - bN) = (x + pxR) - pbN = axN - pbN = (ax - pb)N。
よってNで割り切れる。

## Th 2. y/N ≡ x N^(-1) % pである。

y/N = ax - pbでa ≡ N^(-1) % pなので成り立つ。

## Th 3. 0 <= x < pNのときy/N - ((xN^(-1)) % p) = 0 or p。

0 <= x < pNだから0 =< y < x + pN < 2pN。よって0 < y/N < 2p。
よって成り立つ。
