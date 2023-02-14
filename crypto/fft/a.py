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

def getWs(w):
  ws = [1]
  for i in range(N):
    ws.append((ws[i] * w) % r)
  return ws

def sub(s):
  return s
  s = str(s)
  n = len(s)
  N = 3
  if n < N * 2: return s
  return s[0:N] + "..." + s[-(N-1):]

def PUT(msg, s):
  print(msg)
  if type(s) == list:
    for e in s:
      print(sub(e))
    return
  print(s)

if False:
  root = findGenerator(q, ilogN, r)
else:
  N=16
  r=0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001
  q=(r-1)//N
  root=5

g=pow(root,N,r)
w=pow(root,q,r)

invN = invMod(N, r)
ws = getWs(w)

print('invN=', invN)
print('g=', g)
print('g^q=', pow(g,q,r))
print('w=', w)
print('w^N=', pow(w,N,r))
for i in range(N):
  print(f'w^{i}={pow(w,i,r)}')

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


def rev(n, bitN):
    ret = 0
    for i in range(bitN):
        if (n >> i) & 1:
            ret |= 1 << (bitN - 1 - i)
    return ret

def revArray(xs, bitN):
    for i in range(len(xs)):
        j = rev(i, bitN)
        if j > i:
          xs[i], xs[j] = xs[j], xs[i]

def NTT(xs, ws, rev=False):
  n = len(xs)
  bitN = int(math.log2(n))
  revArray(xs, bitN)
  M = 2
  for _ in range(bitN):
    for i in range(0, n, M):
      g = 0
      for j in range(M >> 1):
         k = i + j + (M >> 1)
         U = xs[i + j]
         gg = n - g if rev else g
         V = xs[k] * ws[gg]
         xs[i + j] = (U + V) % r
         xs[k] = (U - V) % r
         g += n  // M
    M <<= 1
  if rev:
    for i in range(n):
      xs[i] *= invN

def iNTT(xs, ws):
  NTT(xs, ws, True)

#############################################################################

xs = []
for i in range(N):
  v = (i+1)*7
  xs.append(v%r)

PUT('xs', xs)
ys = FFT(xs)
PUT('ys', ys)
zs = iFFT(ys)
PUT('zs', zs)
print('iFFT(FFT(xs)) == xs?', xs == zs)

zs = []
for i in range(N):
  zs.append((ys[i] * pow(w,-i,r))%r)

print('iFFT(zs)', iFFT(zs))

xs2 = xs.copy()
ws = getWs(w)
print('ws', ws)
NTT(xs2, ws)
PUT('xs2', xs2)
print('xs2 == ys?', xs2 == ys)
iNTT(xs2, ws)
PUT('iNTT xs2', xs2)
print('xs2 == xs?', xs2 == xs)

#############################################################################

# https://cgyurgyik.github.io/posts/2021/04/brief-introduction-to-ntt/
def cooley_tukey_ntt_opt(a, n, q, phis):
    """Cooley-Tukey DIT algorithm with an extra optimization.
    We can avoid computing bit reversed order with each call by
    pre-computing the phis in bit-reversed order.
    Requires:
     `phis` are provided in bit-reversed order.
     `n` is a power of two.
     `q` is equivalent to `1 mod 2n`.
    Reference:
       https://www.microsoft.com/en-us/research/wp-content/uploads/2016/05/RLWE-1.pdf
    """

    assert q % (2 * n) == 1, f'{q} is not equivalent to 1 mod {2 * n}'
    assert (n & (n - 1) == 0) and n > 0, f'n: {n} is not a power of 2.'

    t = n
    m = 1
    while m < n:
        t >>= 1
        for i in range(0, m):
            j1 = i * (t << 1)
            j2 = j1 + t - 1
            S = phis[m + i]
            for j in range(j1, j2 + 1):
                U = a[j]
                V = a[j + t] * S
                a[j] = (U + V) % q
                a[j + t] = (U - V) % q
        m <<= 1
    return a


def gentleman_sande_intt_opt(a, n, q, inv_phis):
    """Gentleman-Sande INTT butterfly algorithm.
    Assumes that inverse phis are stored in bit-reversed order.
    Reference:
       https://www.microsoft.com/en-us/research/wp-content/
       uploads/2016/05/RLWE-1.pdf
    """
    t = 1
    m = n
    while (m > 1):
        j1 = 0
        h = m >> 1
        for i in range(h):
            j2 = j1 + t - 1
            S = inv_phis[h + i]
            for j in range(j1, j2 + 1):
                U = a[j]
                V = a[j + t]
                a[j] = (U + V) % q
                a[j + t] = ((U - V) * S) % q
            j1 += (t << 1)
        t <<= 1
        m >>= 1

    shift_n = int(math.log2(n))
    return [(i >> shift_n) % q for i in a]

def get_bit_reversed(c, n):
    cc = c.copy()
    for i in range(n):
        rev_i = rev(i, n.bit_length() - 1)
        if rev_i > i:
            cc[i], cc[rev_i] = cc[rev_i], cc[i]

    return cc

def gen_phis(ws, q):
    def legendre(x, q):
        return pow(x, (q - 1) // 2, q)

    def tonelli_shanks(x, q):
        # Finds the `sqrt(x) mod q`.
        # Source: https://rosettacode.org/wiki/Tonelli-Shanks_algorithm
        Q = q - 1
        s = 0
        while Q % 2 == 0:
            Q //= 2
            s += 1
        if s == 1:
            return pow(x, (q + 1) // 4, q)
        for z in range(2, q):
            if q - 1 == legendre(z, q):
                break
        c = pow(z, Q, q)
        r = pow(x, (Q + 1) // 2, q)
        t = pow(x, Q, q)
        m = s
        t2 = 0
        while (t - 1) % q != 0:
            t2 = (t * t) % q
            for i in range(1, m):
                if (t2 - 1) % q == 0:
                    break
                t2 = (t2 * t2) % q
            b = pow(c, 1 << (m - i - 1), q)
            r = (r * b) % q
            c = (b * b) % q
            t = (t * c) % q
            m = i
        return r

    return [tonelli_shanks(x, q) for x in ws]

"""
phis = gen_phis(ws, r)
inv_phis = get_bit_reversed(phis, N)

xs2 = xs.copy()
cooley_tukey_ntt_opt(xs2, N, r, phis)
PUT('ys', ys)
PUT('xs2', xs2)

gentleman_sande_intt_opt(xs2, N, r, inv_phis)
PUT('xs2', xs2)
"""