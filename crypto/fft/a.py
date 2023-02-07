import random
import math

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

#q, r = N * q+1 are prime
def findPrime(N, start):
  for q in range(start, start+100):
    if isPrime(q):
      r = q * N + 1
      if isPrime(r):
        return (q, r)

ilogN=4
N=2**ilogN
(q, r) = findPrime(N, 300)
print(f'q={q}({isPrime(q)}), r={r}({isPrime(r)}), N={N}')

# sum [g^i : i = 0, ..., N-1]
def sumPowSeq(g, r, show=False):
  s = 0
  x = 1
  v = []
  for i in range(N):
    s += x
    v.append(x)
    x = (x * g) % r
  if show:
    print(v)
  return s % r

def invMod_slow(x, r):
  for i in range(r):
    if (i * x)%r == 1:
      return i

def invMod(x, m):
  if x == 1:
    return 1
  a = 1
  (q, t) = divmod(m, x)
  s = x
  b = -q
  while True:
    (q, s) = divmod(s, t)
    if s == 0:
      if b < 0:
        b += m
      return b
      a -= b * q
      (q, t) = divmod(t, s)
      if t == 0:
        if a < 0:
          a += m
        return a
      b -= a * q

def old_findGenerator(n, r):
  for g in range(3,10000):
   x = 1
   s={x}
   for j in range(n):
     s.add(x)
     x = (x * g) % r
   if x == 1 and len(s) == n:
     return g
  else:
    raise Exception('not found')

# r = q * 2^m + 1
def findGenerator(q, m, r):
  for g in range(3, 1000, 2):
    v = math.gcd(g, q)
    if v == 1:
      return g

if False:
  invN = invMod(N, r)
  root = findGenerator(q, ilogN, r)
  g = pow(root,N,r)
  w = pow(root,q,r)
else:
  N=16
  r=0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001
  q=(r-1)//N
  root=5
  g=pow(root,N,r)
  w=pow(root,q,r)
  invN = invMod(N, r)

print('invN=', invN)
print('g=', g)
print('g^q=', pow(g,q,r))
print('w=', w)
print('w^N=', pow(w,N,r))

for i in range(1, N):
  if sumPowSeq(pow(w,i,r), r) != 0:
    raise Exception('bad sum', i)

def FFT(xs):
  ys = []
  for i in range(N):
    v = 0
    for j in range(N):
      v += xs[j] * pow(w,i*j,r)
    ys.append(v%r)
  return ys

def iFFT(xs):
  ys = []
  for i in range(N):
    v = 0
    for j in range(N):
      v += xs[j] * pow(w,-i*j,r)
    ys.append((v*invN)%r)
  return ys

xs = []
for i in range(N):
  v = (i+1)*12345
  xs.append(v%r)

print('xs', xs)
ys = FFT(xs)
print('ys', ys)
zs = iFFT(ys)
print('zs', zs)
print('iFFT(FFT(xs)) == xs?', xs == zs)

zs = []
for i in range(N):
  zs.append((ys[i] * pow(w,-i,r))%r)

print('iFFT(zs)', iFFT(zs))
