import sys
sys.path.append('../')
from s_xbyak import *
import argparse

def assertEq(x, y):
  if x != y:
    raise Exception('not equal', x, y)

def maskTest():
  assertEq(str(rax), 'rax')
  assertEq(str(al), 'al')
  assertEq(str(ecx+eax*4+123), 'ecx+eax*4+123')
  assertEq(str(ecx+eax*8-123), 'ecx+eax*8-123')
  assertEq(str(ecx), 'ecx')
  assertEq(str(xmm1), 'xmm1')
  assertEq(str(ymm2), 'ymm2')
  assertEq(str(zmm3), 'zmm3')
  assertEq(str(xmm1|k2), 'xmm1{k2}')
  assertEq(str(xmm1|k0), 'xmm1')
  assertEq(str(k1|k2), 'k1{k2}')
  assertEq(str(k2|k1), 'k2{k1}')
  assertEq(str(k1|T_z), 'k1{z}')
  assertEq(str(k2|k1|T_z), 'k2{k1}{z}')
  assertEq(str(xmm1|k2|T_z), 'xmm1{k2}{z}')

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('-win', '--win', help='output win64 abi', action='store_true')
  parser.add_argument('-n', '--num', help='max size of Unit', type=int, default=9)
  parser.add_argument('-addn', '--addn', help='max size of add/sub', type=int, default=16)
  parser.add_argument('-gas', '--gas', help='output gas syntax', default=False, action='store_true')
  parser.add_argument('-m', '--mode', help='output asm syntax', default='nasm')
  global param
  param = parser.parse_args()

  setWin64ABI(param.win)
  N = param.num
  addN = param.addn

  init(param.mode)
  segment('text')

  maskTest()

  term()

if __name__ == '__main__':
  main()
