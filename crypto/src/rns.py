# mod in [-p/2, p/2]
MOD_HALF=True
#MOD_HALF=False

B=[101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151]
C=[179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241]
#251, 257, 263, 269, 271, 277, 281,
#283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 

def prod(xs):
  r = 1
  for e in xs:
    r *= e
  return r

def invMod(x, m):
  return pow(x, -1, m)

def mod(x, p):
  r = x % p
  if MOD_HALF:
    if r > p//2:
      r -= p
  return r

def putSS(xs, ss):
  r = []
  for i in range(len(xs)):
    v = xs[i]
    r.append(v)
  print(r)
  
def split(x, ss):
  """
  return [x mod ss[i]]
  """
  r = []
  for p in ss:
    r.append(mod(x, p))
  return r

def CRT(xs, ss):
  """
  recover x from xs
  return x such that x mod ss[i] = xs[i]
  """
  assert(len(xs) == len(ss))
  s = prod(ss)
  x = 0
  for i in range(len(xs)):
    m = s//ss[i]
    r = invMod(m, ss[i])
    # r * ss[i] = 1 mod s//ss[i]
    v = xs[i]
    x += r * m * v
  return mod(x, s)

def conv(xs, C, B):
  """
  change base from C to B
  xs = split(x, C)
  X = x + Qe where e is small
  return split(X, B)
  """
  assert(len(xs) == len(C))
  Q = prod(C)
  X = 0
  for i in range(len(C)):
    q_hat = Q//C[i]
    r = invMod(q_hat, C[i])
    c = mod(xs[i]*r, C[i])
    X += c * q_hat
  return split(X, B)

def modUp(xs, C, B):
  """
  change base from C to B + C
  xs = split(x, C)
  return split(x + Qe, B + C) where e is a small number
  """
  assert(len(xs) == len(C))
  ys = conv(xs, C, B)
  return ys + xs

def modDown(ys, C, B):
  """
  change base from B + C to C
  y = CRT(ys, B + C)
  return split(approximate value of y/P, C)
  """
  K = len(B)
  L = len(C)
  zs = conv(ys[:K], B, C)
  P = prod(B)
  ret = []
  for i in range(L):
    r = invMod(P, C[i])
    r = mod(ys[K+i] - zs[i], C[i])
    ret.append(r)
  return ret

P = prod(B)
Q = prod(C)
D = B + C
PQ = P * Q

print(f'{B=}')
print(f'{C=}')
print(f'{D=}')
print(f'{P=}')
print(f'{Q=}')
print(f'{PQ=}')

def modUpTest():
  print('modUp')
  for i in range(1, 10):
  #  x = P - i * 100
    x = i * 10
    xs = split(x, C)
    ys = modUp(xs, C, B)
    y = CRT(ys, D)
    print(f'{x=} {y=} {mod(y,Q)=} {(y-x)/Q=}')

def modDownTest():
  print('modDown')
  for i in range(1, 10):
    x = i * P * 10 - i
    xs = split(x, B + C)
    ys = modDown(xs, C, B)
    y = CRT(ys, C)
    print(f'{x=} {y=} {y/P=} {x/P=}')

modUpTest()
modDownTest()
