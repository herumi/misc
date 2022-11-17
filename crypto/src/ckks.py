import numpy as np
from numpy.polynomial import Polynomial as poly
from math import *
import secrets
import random

random.seed(1234)

def getRand(n):
  return random.randint(0, n-1)
#  return secrets.randbelow(n)

class Param:
  def __init__(self):
    self.a = 0
    self.M = 0
    self.N = 0
    self.halfN = 0
    self.xi = 0
    self.A = []
    self.invA = None
    self.cycloPoly = None
    self.Delta = 1000
    self.l = 2
    self.L = 3
    self.p = 1000
    self.q0 = 10000
    self.P = 10000
    self.qL = (self.p ** self.L) * self.q0

# g_M : power of two
# g_Delta : scale
# L : max num of mul
def init(M: int):
  global g_
  g_ = Param()
  g_.M = M
  g_.N = M//2
  g_.halfN = g_.N//2
  g_.xi = np.exp(2 * np.pi * 1j / g_.M)

  # g_.A = (a_ij) = (((g_.xi^(2*i + 1))^j)
  for i in range(g_.N):
    root = g_.xi ** (2 * i + 1)
    row = []
    for j in range(g_.N):
      row.append(root ** j)
    g_.A.append(row)
  g_.invA = np.linalg.inv(g_.A)

  # M-th cyclotomic poly : phi_M(X) = X^N + 1
  g_.cycloPoly = poly([1] + [0] * (g_.N-1) + [1])

def get_ql(l:int) -> int:
  return g_.p ** l * g_.q0

# Sigma(p) = (p(g_.xi^(2*i+1)))
def Sigma(p: poly) -> np.array:
  f = []
  for i in range(g_.N):
    f.append(p(g_.A[i][1]))
  return np.array(f)

def invSigma(b: np.array) -> poly:
  return poly(np.dot(g_.invA, b))

# Projection from C^N -> C^(N/2)
def Pi(z: np.array) -> np.array:
  return z[0:g_.halfN]

# convert a in C^(N/2) to [a:reverse(conj(a))] in C^N
def invPi(z: np.array) -> np.array:
  assert z.shape[0] == g_.halfN
  w = z.conjugate()[::-1]
  return np.hstack([z, w])

def getRealPoly(p: poly) -> poly:
  def f(x):
    if x.imag != 0:
      raise('not zero')
    return x.real
  return poly(list(map(f, p.convert().coef)))

# Round up coefficients of a polynomial
def roundCoeff(p: poly) -> poly:
  p = poly(list(map(np.round, p.convert().coef)))
  p = getRealPoly(p)
  return p

def signMod(x, n):
  t = x % n
  if x >= 0:
    return t
  return t - n

# modulo coefficients of a polynomial
def modCoeff(p: poly, m) -> poly:
  def f(x):
    a = signMod(x.real, m)
    b = signMod(x.imag, m)
    if b == 0:
      return a
    return a + b * 1j
  return poly(list(map(f, p.convert().coef)))

def Encode(z: np.array) -> poly:
  assert z.shape[0] == g_.halfN
  # scale
  h = invSigma(invPi(z) * g_.Delta)
  # round
  p = roundCoeff(h)
  # get real part
  p = getRealPoly(p)
  return p

def Decode(m: poly) -> np.array:
  return Pi(Sigma(m) / g_.Delta)

def randZERO():
  return poly([0] * g_.N)

# QQQ : consider Hamming weight h
# reutrn {-1, 0, 1}^N
def randHWT() -> poly:
  a = []
  for i in range(g_.N):
    v = getRand(3) - 1
    a.append(v)
  return poly(a)

# rho = 0.5 (Pr(+1)=Pr(-1)=1/4, Pr(0)=1/2)
def randZO() -> poly:
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
def randDG() -> poly:
  return randPoly(3)

# R_{qL}
def randRQL() -> poly:
  return randPoly(100)

def modPoly(p: poly) -> poly:
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

def Dec(sk, c):
  b, a = c
  t = modPoly(b + a * sk)
  ql = get_ql(g_.l)
  t = modCoeff(t, ql)
  return t

def Add(c1, c2):
  b1, a1 = c1
  b2, a2 = c2
  return (b1 + b2, a1 + a2)

def Mul(c1, c2, ek=None):
  b1, a1 = c1
  b2, a2 = c2
  ql = get_ql(g_.l)
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

def PUT(msg, x):
  if type(x) == tuple:
    for i in range(len(x)):
      s = msg + '.0=' if i == 0 else ' ' * len(msg) + f'.{i}='
      print(s, x[i])
    return
  print(msg + '=', x)

def main():
  M = 8
  init(M)

  print('Encode / Decode')
  if M == 8:
    z = np.array([3+4j, 2-1j])
  else:
    z = randPlainText(False)
  print(f'{z=}')
  m = Encode(z)
  print(f'enc={m}')
  print(f'dec={Decode(m)}')

  print('\n\nHWT')
  for i in range(5):
    print(randHWT())

  print('\n\nEncrypt / Decrypt')
  sk, pk, ek = KeyGen()
  PUT('sk', sk)
  PUT('pk', pk)
  PUT('ek', ek)

  PUT('z  ', Decode(m))
  PUT('msg', m)
  c = Enc(pk, m)
  PUT('enc', c)
  d = Dec(sk, c)
  PUT('dec', d)
  PUT('dcd', Decode(d))

  print('\n\nAdd / Mul')
  z1 = randPlainText()
  z2 = randPlainText()
#  if M == 8:
#    z1 = np.array([1 + 2j, 3 + 4j])
#    z2 = np.array([-2 + 4j, -5 + 1j])
  PUT('z1', z1)
  PUT('z2', z2)
  m1 = Encode(z1)
  m2 = Encode(z2)
  PUT('m1', m1)
  PUT('m2', m2)
  org1 = Decode(m1)
  org2 = Decode(m2)
  PUT('org1', org1)
  PUT('org2', org2)
  c1 = Enc(pk, m1)
  c2 = Enc(pk, m2)
  PUT('c1', c1)
  PUT('c2', c2)
  PUT('dec c1', Dec(sk, c1))
  PUT('dec c2', Dec(sk, c2))
  c = Add(c1, c2)
  PUT('add', c)
  d = Dec(sk, c)
  PUT('dec', d)
  PUT('dcd', Decode(d))
  PUT('org', org1 + org2)
  c = Mul(c1, c2, ek)
  PUT('mul', c)
  d = Dec(sk, c)
  PUT('dec', d)
  PUT('dcd', Decode(d) / g_.Delta)
  PUT('org', org1 * org2)


if __name__ == '__main__':
  main()
