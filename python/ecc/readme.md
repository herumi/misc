# 楕円曲線

## 自己準同型(endmorphism)

- E/Fp : Fp上の楕円曲線(y^2 = x^3 + ax + b)
- Φ : E → E : 有理写像 → 準同型写像


- p = 1 mod 3, p = 3 mod 4とする。
  - x^(p-1) = 1 mod p for x != 0

w^3 - 1 = 0 / Fp とする
(w - 1)(w^2 + w + 1) = 0 → w = (-1 + √(-3))/2
√(-3) = (-3)^(p+1)//4 in Fp

E : y^2 = x^3 + b / Fpについて
Φ((x, y)) = (wx, y)とすると、Φは準同型写像


secp256k1という楕円曲線のパラメータ
p=0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f ; prime
a=0
b=7
r = #E(Fp) = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141 ; prime
r = E[r] = { P in E(Fp) | r P = 0 }

E(Fp)の生成元をP_0とするとΦ(P_0) in <P_0>なのでL in FrでΦ(P_0) = L P_0となる。
L^3 = 1 mod r.
Φの準同型性から任意のP in
Φ((x, y)) = L(x, y) for (x, y)

L倍は簡単に求められる。

-----------------------------------------------------------------------------
r=0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
w=pow(p-3,(p+1)//4,p)
w=0xa2d2ba93507f1df233770c2a797962cc61f6d15da14ecd47d8d27ae1cd5f852
(w*w+3)%p
0
ww=(-(w+1)//2)%p
hex(ww)
'0xfae96a2b657c07106e64479eac3434e99cf0497512f58995c1396c27f1950005'
B00=0x3086d221a7d46bcde86c90e49284eb15
B01=-0xe4437ed6010e88286f547fa90abfe4c3
B10=0x114ca50f7a8e2f3f657c1108d9d44cfd8
>>> B11=B00
(B00*B11-B10*B01)%r
0
L=0x5363ad4cc05c30e0a5261c028812645a122e22ea20816678df02967c1b23bd72
(L*L*L)%r
1
-----------------------------------------------------------------------------
