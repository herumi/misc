import sys
sys.path.append('../')
from s_xbyak import *
import argparse

def assertEq(x, y):
  if x != y:
    raise Exception('not equal', x, y)

def maskTest():
  # for nasm/masm
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

def miscTest():
  vaddpd(zmm2, zmm5, zmm30)
  vaddpd(xmm30, xmm20, ptr(rax))
  vaddps(xmm30, xmm20, ptr(rax))
  vaddpd(zmm2 | k5, zmm4, zmm2)
  vaddpd(zmm2 | k5 | T_z, zmm4, zmm2)
  vaddpd(zmm2 | k5 | T_z, zmm4, zmm2 | T_rd_sae)
  vaddpd(zmm2 | k5 | T_z | T_rd_sae, zmm4, zmm2)
  vcmppd(k4 | k3, zmm1, zmm2 | T_sae, 5)
  vcmpnltpd(k4|k3,zmm1,zmm2|T_sae)
  vmovups(xm2|k1|T_z, ptr(rax))
  vcvttsh2usi(r9, xmm1|T_sae)
  vcvttph2qq(zmm1|k5|T_z, xmm3|T_sae)

  vaddpd(xmm1, xmm2, ptr (rax+256))
  vaddpd(xmm1, xmm2, ptr_b (rax+256))
  vaddpd(ymm1, ymm2, ptr_b (rax+256))
  vaddpd(zmm1, zmm2, ptr_b (rax+256))
  vaddps(zmm1, zmm2, ptr_b (rax+rcx*8+8))

def runTest():
  pass

def main():
  # before calling init()
  maskTest()

  parser = getDefaultParser()
  global param
  param = parser.parse_args()

  init(param)
  segment('text')

  runTest()

  miscTest()

  term()

if __name__ == '__main__':
  main()
