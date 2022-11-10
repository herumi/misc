import numpy as np
from numpy.polynomial import Polynomial as poly
from math import *

g_M:int = 0
g_N:int = 0
g_xi:np.complex128 = 0
g_A:list = []
g_invA:np.ndarray = None
g_polyMod:poly = None

g_Delta:int = 64
g_L:int = 0
g_p:int = 0
g_q0:int = 0
g_qL:int = 0

# g_M : power of two
# g_Delta : scale
# L : max num of mul
def init(M: int, Delta:int = 64, L:int = 3, p:int = 100, q0:int = 100):
  global g_M
  global g_N
  global g_xi
  g_M = M
  g_N = M // 2
  g_xi = np.exp(2 * np.pi * 1j / g_M)

  # g_A = (a_ij) = (((g_xi^(2*i + 1))^j)
  global g_A
  global g_invA
  for i in range(g_N):
    root = g_xi ** (2 * i + 1)
    row = []
    for j in range(g_N):
      row.append(root ** j)
    g_A.append(row)
  g_invA = np.linalg.inv(g_A)

  global g_polyMod
  g_polyMod = poly([1] + [0] * (g_N-1) + [1])

  global g_Delta, g_L, g_p, g_q0, g_qL
  g_Delta = Delta
  g_L = L
  g_p = p
  g_q0 = q0
  g_qL = (p ** L) * q0

# Sigma(p) = (p(g_xi^(2*i+1)))
def Sigma(p: poly) -> np.array:
  f = []
  for i in range(g_N):
    f.append(p(g_A[i][1]))
  return np.array(f)

def invSigma(b: np.array) -> poly:
  return poly(np.dot(g_invA, b))

def main():
  init(8)

  m1 = np.array([1, 2, 3, 4])
  m2 = np.array([3, -5, 7, -9+2j])
  print(m1)
  print(m2)

  p1 = invSigma(m1)
  p2 = invSigma(m2)

  p_add = p1 + p2
  a = Sigma(p_add)
  print(a)
  p_mult = (p1 * p2) % g_polyMod
  a = Sigma(p_mult)
  print(a)

  print('===')
  m = poly(np.array([10,4*sqrt(2),10,2*sqrt(2)])/4)
  print(m)
  # scaling and round
  m_scale = poly(list(map(round,(m*g_Delta).convert().coef)))
  print(Sigma(m))
  print(Sigma(m_scale)/g_Delta)

if __name__ == '__main__':
  main()
