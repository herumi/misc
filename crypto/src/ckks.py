# -*- coding: utf-8 -*-
import numpy as np
from numpy.polynomial import Polynomial as poly
import secrets
import random

random.seed(1234)

def getRand(n):
  return random.randint(0, n-1)
#  return secrets.randbelow(n)

class Param:
  def __init__(self, delta = 1000):
    self.a = 0
    self.M = 0
    self.N = 0
    self.halfN = 0
    self.xi = 0
    self.S = []
    self.invS = None
    self.cycloPoly = None
    self.delta = delta
    self.L = 3
    self.p = 1000
    self.q0 = 1000
    self.P = 1000
    self.qL = (self.p ** self.L) * self.q0

# g_M : power of two
# g_delta : scale
# L : max num of mul
def init(M: int, delta = 1000):
  global g_
  g_ = Param(delta)
  g_.M = M
  g_.N = M//2
  g_.halfN = g_.N//2
  g_.xi = np.exp(2 * np.pi * 1j / g_.M)

  # g_.S = (a_ij) = (((g_.xi^(2*i + 1))^j)
  for i in range(g_.N):
    root = g_.xi ** (2 * i + 1)
    row = []
    for j in range(g_.N):
      row.append(root ** j)
    g_.S.append(row)
  g_.invS = np.linalg.inv(g_.S)

  # M-th cyclotomic poly : phi_M(X) = X^N + 1
  g_.cycloPoly = poly([1] + [0] * (g_.N-1) + [1])
  return g_

def putVector(v):
  for x in v:
    print(f'{x: 5.2f}', end=' ')
  print()

def putMatrix(a):
  for v in a:
    putVector(v)

def get_ql(l:int):
  return g_.p ** l * g_.q0

# sigma(p) = (p(g_.xi^(2*i+1)))
def sigma(p: poly):
  f = []
  for i in range(g_.N):
    f.append(p(g_.S[i][1]))
  return np.array(f)

def invSigma(b: np.array):
  return poly(np.dot(g_.invS, b))

# Projection from C^N → C^(N/2)
def proj(z: np.array):
  return z[0:g_.halfN]

# convert a in C^(N/2) to [a:reverse(conj(a))] in C^N
def invProj(z: np.array):
  assert z.shape[0] == g_.halfN
  w = z.conjugate()[::-1]
  return np.hstack([z, w])

def getRealPoly(p: poly):
  def f(x):
    if x.imag != 0:
      raise('not zero')
    return x.real
  if type(p) == poly:
    p = p.convert().coef
  return poly(list(map(f, p)))

# Round up coefficients of a polynomial
def roundCoeff(p: poly):
  if type(p) == poly:
    p = p.convert().coef
  p = poly(list(map(np.round, p)))
  return p

def mod(x, n):
  t = x % n
  if t >= n//2:
    t -= n
  return t

# modulo coefficients of a polynomial
def modCoeff(p: poly, m):
  def f(x):
    a = mod(x.real, m)
    b = mod(x.imag, m)
    if b == 0:
      return a
    return a + b * 1j
  return poly(list(map(f, p.convert().coef)))

def Ecd(z: np.array):
  assert z.shape[0] == g_.halfN
  p = invSigma(invProj(z)) * g_.delta
  p = roundCoeff(p)
  p = getRealPoly(p)
  return p

def Dcd(m: poly):
  return proj(sigma(m) / g_.delta)

def randZERO():
  return poly([0] * g_.N)

# QQQ : consider Hamming weight h
# reutrn {-1, 0, 1}^N
def randHWT():
  a = []
  for i in range(g_.N):
    v = getRand(3) - 1
    a.append(v)
  return poly(a)

# rho = 0.5 (Pr(+1)=Pr(-1)=1/4, Pr(0)=1/2)
def randZO():
  a = []
  for i in range(g_.N):
    v = getRand(4)
    if v == 0:
      a.append(-1)
    elif v == 1:
      a.append(1)
    else:
      a.append(0)
  return poly(a)

# (-w, w]
def randPoly(w):
  a = []
  for i in range(g_.N):
    v = getRand(w * 2) - (w - 1)
    a.append(v)
  return poly(a)

# QQQ : consider variance
def randDG():
  return randPoly(3)

# R_{qL}
def randRQL():
  return randPoly(100) # QQQ

def modPoly(p: poly):
  return p % g_.cycloPoly

def randPlainText(rand=True):
  z = []
  for i in range(g_.M // 4):
    if rand:
      v = getRand(10) + getRand(10) * 1j
    else:
      v = i + (i+1) * 1j
    z.append(v)
  return np.array(z)

def KeyGen():
  # secret
  s = randHWT()
  # public
  a = randRQL()
  e = randDG()
  b = modPoly(-a * s + e)
  # evalutate
  n = g_.P * g_.qL
  ap = randRQL()
  ep = randDG()
  bp = modPoly(-ap * s + ep + g_.P * s * s)
  bp = modCoeff(bp, n)
  return (s, (b, a), (bp, ap))

def Enc(pk, m):
  v = randZO()
  e0 = randDG()
  e1 = randDG()
  t0 = modPoly(v * pk[0] + m + e0)
  t1 = modPoly(v * pk[1] + e1)
  t0 = modCoeff(t0, g_.qL)
  t1 = modCoeff(t1, g_.qL)
  t0 = getRealPoly(t0)
  t1 = getRealPoly(t1)
  return (t0, t1)

def Dec(sk, c, l=0):
  b, a = c
  t = modPoly(b + a * sk)
  if l == 0:
    l = g_.L
  ql = get_ql(l)
  t = modCoeff(t, ql)
  return t

def add(c1, c2):
  b1, a1 = c1
  b2, a2 = c2
  return (b1 + b2, a1 + a2)

def mul(c1, c2, ek=None, l=0):
  b1, a1 = c1
  b2, a2 = c2
  if l == 0:
    l = g_.L
  ql = get_ql(l)
  d0 = modPoly(b1 * b2)
  d1 = modPoly(a1 * b2 + a2 * b1)
  d2 = modPoly(a1 * a2)
  t0 = modPoly(d2 * ek[0])
  t1 = modPoly(d2 * ek[1])
  t0 = roundCoeff(t0 / g_.P)
  t1 = roundCoeff(t1 / g_.P)
  t0 = d0 + roundCoeff(t0)
  t1 = d1 + roundCoeff(t1)
  t0 = modCoeff(t0, ql)
  t1 = modCoeff(t1, ql)
  return (t0, t1)

def rescale(c, l, lp):
  """
  convert c in R_{q^l} to R_{q^lp}
  """
  t0, t1= c
  d = g_.p ** (l - lp)
  t0 = roundCoeff(t0 / d)
  t1 = roundCoeff(t1 / d)
  return (t0, t1)

# X → X^k
def Permutate(f, k):
  co = f.convert().coef
  g = [co[0]]
  for i in range(1, len(co)):
    g.extend([0] * (k-1))
    g.append(co[i])
  return modPoly(g)

def PUT(msg, x):
  if type(x) == tuple:
    for i in range(len(x)):
      s = msg + '.0=' if i == 0 else ' ' * len(msg) + f'.{i}='
      print(s, x[i])
    return
  print(msg + '=', x)

def main():
  print('Ecd / Dcd for M=8')
  M = 8
  init(M, delta=64)
  if M == 8:
    z = np.array([3+4j, 2-1j])
  else:
    z = randPlainText(False)
  print(f'{z=}')
  m = Ecd(z)
  print(f'ecd={m}')
  print(f'ded={Dcd(m)}')

  M = 8
  init(M)

  print('\n\nHWT')
  for i in range(5):
    print(randHWT())

  print('\n\nEncrypt / Decrypt')
  sk, pk, ek = KeyGen()
  PUT('sk', sk)
  PUT('pk', pk)
  PUT('ek', ek)

  PUT('z  ', Dcd(m))
  PUT('msg', m)
  c = Enc(pk, m)
  PUT('enc', c)
  d = Dec(sk, c)
  PUT('dec', d)
  PUT('dcd', Dcd(d))

  print('\n\nAdd / mul')
  z1 = randPlainText()
  z2 = randPlainText()
#  if M == 8:
#    z1 = np.array([1 + 2j, 3 + 4j])
#    z2 = np.array([-2 + 4j, -5 + 1j])
  PUT('z1', z1)
  PUT('z2', z2)
  m1 = Ecd(z1)
  m2 = Ecd(z2)
  PUT('m1', m1)
  PUT('m2', m2)
  PUT('dcd1', Dcd(m1))
  PUT('dcd2', Dcd(m2))
  c1 = Enc(pk, m1)
  c2 = Enc(pk, m2)
  PUT('c1', c1)
  PUT('c2', c2)
  PUT('dec c1', Dec(sk, c1))
  PUT('dec c2', Dec(sk, c2))
  PUT('dcd c1', Dcd(Dec(sk, c1)))
  PUT('dcd c2', Dcd(Dec(sk, c2)))
  c = add(c1, c2)
  PUT('add', c)
  d = Dec(sk, c)
  PUT('dec', d)
  PUT('dcd', Dcd(d))
  PUT('org', z1 + z2)
  c = mul(c1, c2, ek)
  PUT('mul', c)
  d = Dec(sk, c)
  PUT('dec', d)
  PUT('dcd', Dcd(d) / g_.delta)
  PUT('org', z1 * z2)
  c3 = mul(c, c2, ek)
  PUT('c3', c3)
  PUT('dcd c3', Dcd(Dec(sk, c3))/g_.delta**2)
  PUT('org', z1 * z2 * z2)
#  l = g_.L
#  cc = rescale(c3, l, l-1)
#  PUT('cc', cc)
#  PUT('dcd cc', Dcd(Dec(sk, cc))/g_.delta)


if __name__ == '__main__':
  main()
