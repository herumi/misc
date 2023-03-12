from s_xbyak_llvm import *
import argparse

unit = 64

def gen_addPre(N):
  bit = unit * N
  resetGlobalIdx()
  r = Operand(Int, unit)
  x = Operand(Int, unit)
  y = Operand(Int, unit)
  with Function(f'addPre{N}', r, x, y):
    r = add(x, y)
    ret(r)
    pass
  """
  pz = Operand(IntPtr, unit)
  px = Operand(IntPtr, unit)
  py = Operand(IntPtr, unit)
  name = f'mcl_fp_addPre{N}'
  with Function(name, r, pz, px, py):
    x = zext(loadN(px, N), bit + unit)
    y = zext(loadN(py, N), bit + unit)
    z = add(x, y)
    storeN(trunc(z, bit), pz)
    r = trunc(lshr(z, bit), unit)
    ret(r)
  """

def main():
  gen_addPre(4)
  term()

if __name__ == '__main__':
  main()
