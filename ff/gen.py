from s_xbyak_llvm import *
import argparse

unit = 64

def func1(N):
  bit = unit * N
  resetGlobalIdx()
  z = Int(unit)
  x = Int(unit)
  y = Int(unit)
  name = f'func1{N}'
  with Function(name, z, x, y):
    z = add(x, y)
    ret(z)

def gen_addPre(N):
  bit = unit * N
  resetGlobalIdx()
  pz = IntPtr(unit)
  px = IntPtr(unit)
  py = IntPtr(unit)
  name = f'mcl_fp_addPre{N}'
  with Function(name, Void, pz, px, py):
#    x = zext(loadN(px, N), bit + unit)
#    y = zext(loadN(py, N), bit + unit)
#    z = add(x, y)
#    storeN(trunc(z, bit), pz)
#    r = trunc(lshr(z, bit), unit)
    ret(Void)

def main():
  func1(4)
  gen_addPre(4)
  term()

if __name__ == '__main__':
  main()
