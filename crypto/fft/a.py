import random

# return (r, d) such that x = 2^r d where d is odd
def decomposeOdd(x):
  r = 0
  d = x
  while (d % 2) == 0:
    d >>= 1
    r += 1
  return (r, d)

def isPrime(n, tryNum = 32):
  if n <= 1:
    return False
  if n == 2 or n == 3:
    return True
  if (n % 2) == 0:
    return False
  # n - 1 = 2^r d
  (r, d) = decomposeOdd(n-1)
  for i in range(tryNum):
    a = random.randint(2, n-1)
    x = pow(a, d, n)
    if x == 1 or x == n-1:
      continue
    for j in range(1, r+1):
      x = (x * x) % n
      if x == 1:
        return False
      if x == n-1:
        break
    else:
      return False
  return True

#p = 1019, q = 32p+1=32609 are prime
def find(N, start):
  for p in range(start, start+100):
    if isPrime(p):
      q = p * N + 1
      if isPrime(q):
        return (p, q)

N=16
(p, q) = find(N, 40)
print(f'p={p}({isPrime(p)}), q={q}({isPrime(q)}), N={N}')

# sum [g^i : i = 0, ..., N-1]
def sumPowSeq(g, N, show=False):
  s = 0
  x = 1
  v = []
  for i in range(N):
    s += x
    v.append(x)
    x = (x * g) % q
  if show:
    print(v)
  return s % q

def findGenerator(N, q):
  for g in range(1,q):
    x = 1
    s={x}
    for j in range(N):
      s.add(x)
      x = (x * g) % q
    if x == 1 and len(s) == N:
      if sumPowSeq((g*g)%q, N) == 0:
        return g
  else:
    raise Exception('not found')

g = findGenerator(N, q)
print('g=', g)
print('g^N=', pow(g,N,q))
print('sumPowSeq=', sumPowSeq(g, N, True))

def FFT(xs):
  ys = []
  for i in range(N):
    v = 0
    for j in range(N):
      v += xs[j] * pow(g,i*j,q)
    ys.append(v%q)
  return ys

def iFFT(xs):
  ys = []
  for i in range(N):
    v = 0
    for j in range(N):
      v += xs[j] * pow(g,(N-0-i)*j,q)
    ys.append((v%q)//N)
  return ys


xs = []
for i in range(N):
  xs.append((i*i+3*i+1)%N)

print('xs', xs)
ys = FFT(xs)
print('ys', ys)
print('iFFT(FFT(xs)) == xs?', xs == iFFT(ys))
