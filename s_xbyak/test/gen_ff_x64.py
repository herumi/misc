import sys
sys.path.append('../')
sys.path.append('../../../mcl-ff/')
from s_xbyak import *
from primetbl import *
from mont import *
import argparse

unit = 0
unit2 = 0
MASK = 0
mont = None

def setGlobalParam(opt):
  global unit, unit2, MASK
  unit = opt.u
  unit2 = unit * 2
  MASK = (1 << unit) - 1

  global mont
  mont = Montgomery(opt.p, unit)

primeTbl = {
  'BLS12-381-p' : 0x1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab,
  'BLS12-381-r' : 0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001,
  'BN254-p' : 0x2523648240000001ba344d80000000086121000000000013a700000000000013,
  'BN254-r' : 0x2523648240000001ba344d8000000007ff9f800000000010a10000000000000d,
  'p511' : 0x65b48e8f740f89bffc8ab0d15e3e4c4ab42d083aedc88c425afbfcc69322c9cda7aac6c567f35507516730cc1f0b4f25c2721bf457aca8351b81b90533c6c87b,
}

# add(x, y) if noCF is True
# adc(x, y) if noCF is False
def add_ex(x, y, noCF):
  if noCF:
    add(x, y)
  else:
    adc(x, y)

# sub(x, y) if noCF is True
# sbb(x, y) if noCF is False
def sub_ex(x, y, noCF):
  if noCF:
    sub(x, y)
  else:
    sbb(x, y)

def getAt(x, i):
  if type(x) == list:
    return x[i]
  if type(x) == tuple:
    (r, m) = x
    if i < len(r):
      return r[i]
    else:
      return ptr(m + 8 * i)
  raise Exception(f'bad type={type(x)} x={x}, i={i}')

def getNum(x):
  if type(x) == Reg:
    return 1
  if type(x) == Address:
    return 1
  if type(x) == list:
    return len(x)
  if type(x) == tuple:
    (r, m) = x
    return len(r)
  raise Exception(f'bad type={type(x)} x={x}, i={i}')

def make_vec_pm(op, x, addr):
  for i in range(len(x)):
    op(getAt(x, i), ptr(addr + 8 * i))

def make_vec_pp(op, x, y):
  for i in range(len(x)):
#    op(getAt(x, i), getAt(y, i))
    op(x[i], y[i])

# [addr] = x[]
def store_mp(addr, x):
  for i in range(len(x)):
    mov(ptr(addr + 8 * i), x[i])

def load_pm(x, addr):
  make_vec_pm(mov, x, addr)

def mov_pp(x, y):
  make_vec_pp(mov, x, y)

def cmovc_pp(x, y):
  make_vec_pp(cmovc, x, y)

def sub_pm(x, addr):
  n = len(x)
  for i in range(n):
    sub_ex(x[i], ptr(addr + i * 8), i == 0)

def gen_add(name, mont):
  N = mont.pn
  align(16)
  with FuncProc(name):
    assert not mont.isFullBit
    n = min(N*2-2, 11)
    with StackFrame(3, n) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      t1 = sf.t[0:N]
      for i in range(N):
        mov(t1[i], ptr(px + i * 8))
        add_ex(t1[i], ptr(py + i * 8), i == 0)
      t2 = sf.t[N:]
      t2.append(px)
      t2.append(py)
      assert len(t2) == N
      for i in range(N):
        mov(t2[i], t1[i])
        mov(rax, (mont.p >> (i*64))%(2**64))
        sub_ex(t2[i], rax, i == 0)
      for i in range(N):
        cmovc(t2[i], t1[i])
        mov(ptr(pz + i * 8), t2[i])

def gen_sub(name, mont):
  N = mont.pn
  align(16)
  with FuncProc(name):
    assert not mont.isFullBit
    n = min(N*2-2, 11)
    with StackFrame(3, n) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      t1 = sf.t[0:N]
      for i in range(N):
        mov(t1[i], ptr(px + i * 8))
        sub_ex(t1[i], ptr(py + i * 8), i == 0)
      sbb(rax, rax) # -1 if x<y else 0
      t2 = sf.t[N:]
      t2.append(px)
      t2.append(py)
      assert len(t2) == N
      # t2 = p if x<y else 0
      for i in range(N):
        mov(t2[i], (mont.p >> (i*64))%(2**64))
        and_(t2[i], rax)
      for i in range(N):
        add_ex(t1[i], t2[i], i == 0)
        mov(ptr(pz + i*8), t1[i])

#  c[n..0] = c[n-1..0] + px[n-1..0] * rdx if is_cn_zero = True
#  c[n..0] = c[n..0] + px[n-1..0] * rdx if is_cn_zero = False
#  use rdx, t, t2
def mulAdd(c, px, t, t2, is_cn_zero):
  n = len(c)-1
  if is_cn_zero:
    xor_(c[n], c[n])
  else:
    xor_(t, t) # clear ZF
  for i in range(n):
    mulx(t, t2, ptr(px + i * 8))
    adox(c[i], t2)
    if i == n-1:
      break
    adcx(c[i + 1], t)
  adox(c[n], t)
  adc(c[n], 0)

#  c[n..0] = px[n-1..0] * rdx
#  use t
def mulPack1(c, px, t):
  n = len(c)-1
  mulx(c[1], c[0], ptr(px + 0 * 8))
  for i in range(1, n):
    mulx(c[i + 1], t, ptr(px + i * 8))
    add_ex(c[i], t, i == 1)
  adc(c[n], 0)

def montgomery1(mont, c, px, pp, t1, t2, isFirst):
  d = rdx
  if isFirst:
    # c[n..0] = px[n-1..0] * rdx
    mulPack1(c, px, t1)
  else:
    # c[n..0] = c[n-1..0] + px[n-1..0] * rdx because of not fuill bit
    mulAdd(c, px, t1, t2, True)

  mov(d, mont.ip)
  imul(d, c[0]) # d = q = uint64_t(d * c[0])
  # c[n..0] += p * q because of not fuill bit
  mulAdd(c, pp, t1, t2, False)

def rotatePack(pk):
  t = pk[1:]
  t.append(pk[0])
  return t

# Montgomery mul(x, y)
def gen_mul(name, mont):
  N = mont.pn
  align(16)
  with FuncProc(name):
    assert not mont.isFullBit
    with StackFrame(3, N+3, useRDX=True) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      pk = sf.t[0:N+1]
      t = sf.t[N+1]
      t2 = sf.t[N+2]

      lea(rax, rip('PRE(p)'))
      for i in range(N):
        mov(rdx, ptr(py + i * 8))
        montgomery1(mont, pk, px, rax, t, t2, i == 0)
        if i < N - 1:
          pk = rotatePack(pk)
      keep = [pk[0], px, py, rdx]
      pk = pk[1:]
      keep.extend(sf.t[N+1:])
      keep = keep[0:N]
      assert len(keep) == N
      mov_pp(keep, pk)
      sub_pm(pk, rax) # z - p
      cmovc_pp(pk, keep)
      store_mp(pz, pk)

def main():
  parser = argparse.ArgumentParser(description='gen bint')
  parser.add_argument('-p', type=str, default='', help='characteristic of a finite field')
  parser.add_argument('-type', type=str, default='BLS12-381-p', help='elliptic curve type')
  parser.add_argument('-pre', type=str, default='mcl_fp_', help='prefix of a function name')
  parser.add_argument('-win', '--win', help='output win64 abi', action='store_true')
  parser.add_argument('-gas', '--gas', help='output gas syntax', default=False, action='store_true')
  parser.add_argument('-m', '--mode', help='output asm syntax', default='nasm')
  opt = parser.parse_args()

  setWin64ABI(opt.win)
  init(opt.mode)
  opt.u = 64
  opt.proto = False
  if opt.p == '':
    opt.p = primeTbl[opt.type]

  setGlobalParam(opt)
  if opt.proto:
    showPrototype()

  segment('data')
  makeVar('p', mont.bit, mont.p, const=True, static=True)
  makeVar('zero', mont.bit, 0, const=True, static=True)
  makeVar('ip', unit, mont.ip, const=True, static=True)
  segment('text')

  name = f'{opt.pre}add'
  gen_add(name, mont)
  name = f'{opt.pre}sub'
  gen_sub(name, mont)
  name = f'{opt.pre}mul'
  gen_mul(name, mont)

  term()

if __name__ == '__main__':
  main()
