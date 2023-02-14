import sys
sys.path.append('../../mcl/src/')
from s_xbyak import *
import argparse

def gen_add(N):
  align(16)
  with FuncProc(f'mclb_add{N}'):
    with StackFrame(3) as sf:
      z = sf.p[0]
      x = sf.p[1]
      y = sf.p[2]
      for i in range(N):
        mov(rax, ptr(x + 8 * i))
        if i == 0:
          add(rax, ptr(y + 8 * i))
        else:
          adc(rax, ptr(y + 8 * i))
        mov(ptr(z + 8 * i), rax)
      setc(al)
      movzx(eax, al)

# t[] = [px] + [py]
def add_rmm(t, px, py):
  for i in range(len(t)):
    mov(t[i], ptr(px + 8 * i))
    if i == 0:
      add(t[i], ptr(py + 8 * i))
    else:
      adc(t[i], ptr(py + 8 * i))

def vec_n(op, x, y, n):
  for i in range(n):
    op(getAt(x, i), getAt(y, i))

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

def vec_rm(op, x, addr):
  for i in range(len(x)):
    op(x[i], ptr(addr + 8 * i))

def vec_mr(op, addr, x):
  for i in range(len(x)):
    op(ptr(addr + 8 * i), x[i])

# t[] -= [px]
def sub_rm(t, px):
  for i in range(len(t)):
    if i == 0:
      sub(t[i], ptr(px + 8 * i))
    else:
      sbb(t[i], ptr(px + 8 * i))

# [addr] = x[]
def store(addr, x):
  vec_mr(mov, addr, x)

def gen_fp_add(N):
  align(16)
  with FuncProc(f'mclb_fp_add{N}'):
    n = min(N*2-2,10)
    with StackFrame(4, n) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      pp = sf.p[3]
      X = sf.t[0:N]
      T = sf.t[N:]
      add_rmm(X, px, py) # X = px[] + py[]
      T.append(px)
      T.append(py)
      vec_n(mov, (T, pz), X, N)
      sub_rm(X, pp)      # X -= pp[]
      for i in range(N):
        cmovc(X[i], getAt((T, pz), i))
        mov(ptr(pz + 8 * i), X[i])
#      vec_n(cmovc, X, (T, pz), N)
#      store(pz, X)

parser = argparse.ArgumentParser()
parser.add_argument('-win', '--win', help='output win64 abi', action='store_true')
parser.add_argument('-m', '--mode', help='output asm syntax', default='nasm')
param = parser.parse_args()

setWin64ABI(param.win)
init(param.mode)

segment('text')
gen_fp_add(8)
term()
