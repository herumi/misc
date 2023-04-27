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
  parser = getDefaultParser()
  global param
  param = parser.parse_args()

  init(param)
  segment('text')

  vmovups(xm2|k1|T_z, ptr(rax))
  vcvttsh2usi(r9, xmm1|T_sae)
  vcvttph2qq(zmm1|k5|T_z, xmm3|T_sae)
#  maskTest()

  term()

if __name__ == '__main__':
  main()
