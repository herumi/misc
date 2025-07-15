import math

class ConstDiv:
  def __init__(self, d, M=2**32-1):
    self.d = d
    self.M = M
    M_d = M - ((M+1)%d)
    a = math.ceil(math.log2(d))
    while True:
      A = 1 << a
      c = (A + d - 1) // d
      e = d * c - A
      if e * M_d < A:
        self.a = a
        self.A = A
        self.c = c
        self.e = e
        return
      a += 1

  def divd(self, x):
    return (x * self.c) >> self.a

  def __str__(self):
    return f'd={self.d} c={self.c} c.len={self.c.bit_length()} a={self.a} e={self.e}'


M=2**9-1

def findMax(M):
  print(f'{M=} len={M.bit_length()}')
  maxc = 0
  maxcd = None
  for d in range(1, M+1):
    cd = ConstDiv(d, M)
    if cd.c > maxc:
      maxc = cd.c
      maxcd = cd

  print(f'{maxcd}')


for i in range(8, 18):
  M=2**i-1
  findMax(M)  
