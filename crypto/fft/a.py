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

#q, p = N * q+1 are prime
def find(N, start):
  for q in range(start, start+100):
    if isPrime(q):
      p = q * N + 1
      if isPrime(p):
        return (q, p)

N=16
(q, p) = find(N, 100)
print(f'q={q}({isPrime(q)}), p={p}({isPrime(p)}), N={N}')

# sum [g^i : i = 0, ..., N-1]
def sumPowSeq(g, p, show=False):
  s = 0
  x = 1
  v = []
  for i in range(N):
    s += x
    v.append(x)
    x = (x * g) % p
  if show:
    print(v)
  return s % p

def inv(x, p):
  for i in range(p):
    if (i * x)%p == 1:
      return i

def findGenerator(n, p):
  for g in range(3,100):
    x = 1
    s={x}
    for j in range(n):
      s.add(x)
      x = (x * g) % p
    if x == 1 and len(s) == n:
      return g
  else:
    raise Exception('not found')

invN = inv(N, p)
print('invN=', invN)
g = findGenerator(q, p)
print('g=', g)
print('g^q=', pow(g,q,p))
w = findGenerator(N, p)
print('w=', w)
print('w^N=', pow(w,N,p))
for i in range(1, N):
  if sumPowSeq(pow(w,i,p), p) != 0:
    raise Exception('bad sum', i)

def FFT(xs):
  ys = []
  for i in range(N):
    v = 0
    for j in range(N):
      v += xs[j] * pow(w,i*j,p)
    ys.append(v%p)
  return ys

def iFFT(xs):
  ys = []
  for i in range(N):
    v = 0
    for j in range(N):
      v += xs[j] * pow(w,-i*j,p)
    ys.append((v*invN)%p)
  return ys

xs = []
for i in range(N):
  v = (i+1)*12345
  xs.append(v%p)

print('xs', xs)
ys = FFT(xs)
print('ys', ys)
zs = iFFT(ys)
print('zs', zs)
print('iFFT(FFT(xs)) == xs?', xs == zs)

zs = []
for i in range(N):
  zs.append((ys[i] * pow(w,-i,p))%p)

print('iFFT(zs)', iFFT(zs))

