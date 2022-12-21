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

def getText3(a, b):
  n = g_.M // 4
  z = [a + b * 1j] + [0] * (n-1)
  return np.array(z)

for i in range(2, 12):
  M = 2 ** i
  print('M=', M)
  g_ = init(M, Delta = 1)
  a = -M//8
  b = 0
  z = getText3(a, b)
  v = getMaxE(z)
  print(f'v={v}')

"""
for i in range(7, 8):
  M = 2 ** i
  print('M=', M)
  g_ = init(M, Delta = 1)
  m = 0
  ma = 0
  mb = 0
  for a in range(-M,M+1):
    for b in range(0,1):
      z = getText3(a, b)
      v = getMaxE(z)
      if v > m:
        ma = a
        mb = b
        m = v
        print(f'a={a} b={b} v={v}')
  print(f'max={m} a={ma} b={mb}')
"""  
