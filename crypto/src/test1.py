from ckks import *
from numpy.polynomial import Polynomial as poly
import math

M = 512 # 2048
g_ = init(M, Delta = 1)

def getMaxE(z):
  m = Encode(z)
  d = Decode(m)
  max = 0
  for i in range(len(z)):
    v = math.fabs(z[i].real - d[i].real)
    if v > max:
      max = v
    v = math.fabs(z[i].imag - d[i].imag)
    if v > max:
      max = v
  return max

def getText():
  n = g_.M // 4
  z = []
  for i in range(n):
#    v = i * 1j
    v = +(n // 4) + (n // 2) * 1j if i == 0 else 0
    z.append(v)
  return np.array(z)

def getText2():
  a = []
  for i in range(g_.N):
    a.append(0.5 + 0.5j)
  p = poly(a)
  p = Sigma(p)
  p = Pi(p)
  p = roundCoeff(p)
  p = np.array(p.convert().coef)
  print(f'p={p}')
  return p

for i in range(2, 12):
  M = 2 ** i
  print('M=', M)
  g_ = init(M, Delta = 1)
  z = getText()
  print('max', getMaxE(z))
