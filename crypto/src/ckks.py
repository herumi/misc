import numpy as np
from numpy.polynomial import Polynomial as poly
from math import *
import secrets

FIX_RAND = False
sss = 12345

def getRand(n):
  if FIX_RAND:
    global sss
    sss = (123456789 * sss + 9876543) % 0xffffffff
    return sss % n
  return secrets.randbelow(n)

class Param:
  def __init__(self):
    self.a = 0
    self.M = 0
    self.N = 0
    self.halfN = 0
    self.xi = 0
    self.A = []
    self.invA = None
    self.polyMod = None
    self.Delta = 0
    self.L = 0
    self.p = 0
    self.q0 = 0
    self.qL = 0
    self.P = 0

g_ = None

# g_M : power of two
# g_Delta : scale
# L : max num of mul
def init(M: int, Delta:int = 64, L:int = 3, p:int = 100, q0:int = 100, P:int = 100):
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

  g_.polyMod = poly([1] + [0] * (g_.N-1) + [1])

  g_.Delta = Delta
  g_.L = L
  g_.p = p
  g_.q0 = q0
  g_.qL = (p ** L) * q0
  g_.P = P

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

# Round up coefficients of a polynomial
def roundPoly(p: poly) -> poly:
  return poly(list(map(np.round, p.convert().coef)))

# modulo coefficients of a polynomial
def modPoly(p: poly, mod) -> poly:
  def f(x):
    return x # % mod
  return poly(list(map(f, p.convert().coef)))

def Encode(z: np.array) -> poly:
  assert z.shape[0] == g_.halfN
  # scale
  h = invSigma(invPi(z) * g_.Delta)
  # round
  return roundPoly(h)

def Decode(m: poly) -> np.array:
  return Pi(Sigma(m / g_.Delta))

# QQQ : consider Hamming weight h
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
  return randPoly(6)

def KeyGen():
  # secret
  s = randHWT()
  # public
  a = randRQL()
  e = randDG()
  b = -a * s + e
  # evalutate
  ap = randPoly(g_.P * g_.qL)
  ep = randDG()
  bp = (-ap * s + ep + g_.P * s * s) % g_.polyMod
  return (s, (b, a), (bp, ap))

def randPlainText(rand=True):
  z = []
  for i in range(g_.M // 4):
    if rand:
      v = getRand(10) + getRand(10) * 1j
    else:
      v = i + (i+1) * 1j
    z.append(v)
  return np.array(z)

def Enc(pub, m):
  v = randZO()
  e0 = randDG()
  e1 = randDG()
  return ((v * pub[0] + m + e0) % g_.polyMod, (v * pub[1] + e1) % g_.polyMod)

def Dec(sec, c):
  b, a = c
  return (b + a * sec) % g_.polyMod

def Add(c1, c2):
  b1, a1 = c1
  b2, a2 = c2
  return (b1 + b2, a1 + a2)

def Mul(c1, c2, evk=None):
  b1, a1 = c1
  b2, a2 = c2
  m = g_.p * g_.q0
  d0 = modPoly((b1 * b2) % g_.polyMod, m)
  d1 = modPoly((a1 * b2 + a2 * b1) % g_.polyMod, m)
  d2 = modPoly((a1 * a2) % g_.polyMod, m)
  if evk == None:
    return (d0, d1, d2)
  t0 = modPoly(roundPoly(d2 * evk[0] / g_.P) % g_.polyMod, m)
  t1 = modPoly(roundPoly(d2 * evk[1] / g_.P) % g_.polyMod, m)
  return (d0 + t0, d1 + t1)

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
  sec, pub, evk = KeyGen()
  print('sec=', sec)
  PUT('pub', pub)

  PUT('z  ', Decode(m))
  PUT('msg', m)
  c = Enc(pub, m)
  PUT('enc', c)
  d = Dec(sec, c)
  PUT('dec', d)
  PUT('dcd', Decode(d))

  print('\n\nAdd / Mul')
  z1 = randPlainText()
  z2 = randPlainText()
  PUT('z1', z1)
  PUT('z2', z2)
  m1 = Encode(z1)
  m2 = Encode(z2)
  PUT('m1', m1)
  PUT('m2', m2)
  PUT('org1', Decode(m1))
  PUT('org2', Decode(m2))
  c1 = Enc(pub, m1)
  c2 = Enc(pub, m2)
  PUT('c1', c1)
  PUT('c2', c2)
  c = Add(c1, c2)
  PUT('add', c)
  PUT('dec', Dec(sec, c))
  PUT('dcd', Decode(Dec(sec, c)))
  c = Mul(c1, c2, evk)
  PUT('mul', c)
  PUT('dec(c)', Dec(sec, c))
  PUT('dcd(dec(c))', Decode(Dec(sec, c)))


if __name__ == '__main__':
  main()
