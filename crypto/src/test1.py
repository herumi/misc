from ckks import *
from numpy.polynomial import Polynomial as poly
import math

M = 2048
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
  return v

def getText(pos):
  n = g_.M // 4
  z = []
  for i in range(n):
#    v = 30000j if i == pos else 0
    v = i * 1j
    z.append(v)
  return np.array(z)


for pos in range(100):
  z = getText(pos)
  print('max', getMaxE(z))
