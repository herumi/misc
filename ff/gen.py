from s_xbyak_llvm import *
import argparse

unit = 0
unit2 = 0

def setGlobalParam(u):
  global unit, unit2
  unit = u
  unit2 = u * 2

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

def gen_add(N):
  bit = unit * N
  resetGlobalIdx()
  Int(unit)
  pz = IntPtr(unit)
  px = IntPtr(unit)
  py = IntPtr(unit)
  name = f'mcl_fp_addPre{N}'
  with Function(name, Void, pz, px, py):
    x = zext(loadN(px, N), bit + unit)
    y = zext(loadN(py, N), bit + unit)
    z = add(x, y)
    storeN(trunc(z, bit), pz)
    r = trunc(lshr(z, bit), unit)
    ret(Void)

def gen_mulUU():
  resetGlobalIdx();
  z = Int(unit2)
  x = Int(unit)
  y = Int(unit)
  name = f'mul{unit}x{unit}L'
  with Function(name, z, x, y, private=True):
    x = zext(x, unit2)
    y = zext(y, unit2)
    z = mul(x, y)
    ret(z)

def gen_once():
  gen_mulUU()

def main():
  parser = argparse.ArgumentParser(description='gen bint')
  parser.add_argument('-u', type=int, default=64, help='unit')
  parser.add_argument('-n', type=int, default=0, help='max size of unit')
  parser.add_argument('-addn', type=int, default=0, help='mad size of add/sub')
  opt = parser.parse_args()
  if opt.n == 0:
    opt.n = 9 if opt.u == 64 else 17
    opt.addn = 16 if opt.u == 64 else 32

  setGlobalParam(opt.u)

  gen_once()
  func1(4)
  gen_add(3)
  term()

if __name__ == '__main__':
  main()
