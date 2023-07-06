import sys
sys.path.append('../../s_xbyak/')
from s_xbyak import *

def gen_mul_gfni():
  with FuncProc('gf256_mul_gfni'):
    with StackFrame(3, vNum=1, vType=T_YMM) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      vmovups(ymm0, ptr(px))
      vgf2p8mulb(ymm0, ymm0, ptr(py))
      vmovups(ptr(pz), ymm0)

def gen_inv_gfni():
  with FuncProc('gf256_inv_gfni'):
    with StackFrame(2, vNum=2, vType=T_YMM) as sf:
      py = sf.p[0]
      px = sf.p[1]
      vmovups(ymm0, ptr(px))
      vbroadcastsd(ymm1, ptr(rip+'matrixI'))
      # ymm0 * matrixI + 0
      vgf2p8affineinvqb(ymm0, ymm0, ymm1, 0)
      vmovups(ptr(py), ymm0)

def gen_mul_gfni512():
  with FuncProc('gf256_mul_gfni512'):
    with StackFrame(3, vNum=1, vType=T_ZMM) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      vmovups(zmm0, ptr(px))
      vgf2p8mulb(zmm0, zmm0, ptr(py))
      vmovups(ptr(pz), zmm0)

def gen_inv_gfni512():
  with FuncProc('gf256_inv_gfni512'):
    with StackFrame(2, vNum=1, vType=T_ZMM) as sf:
      py = sf.p[0]
      px = sf.p[1]
      vmovups(zmm0, ptr(px))
      # ymm0 * matrixI + 0
      vgf2p8affineinvqb(zmm0, zmm0, ptr_b(rip+'matrixI'), 0)
      vmovups(ptr(py), zmm0)

def gen_pclmulqdq():
  with FuncProc('a_pclmulqdq'):
    with StackFrame(3, vNum=1, vType=T_ZMM) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      # pz[128/8] = px[64/8] * py[64/8]
      mov(rax, ptr(px))
      vmovq(xmm0, rax)
      vpclmulqdq(xmm0, xmm0, ptr(py), 0)
      vmovups(ptr(pz), xmm0)

def main():

  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')
  makeLabel('matrixI')
  dq_(0x0102040810204080)

  gen_mul_gfni()
  gen_inv_gfni()
  gen_mul_gfni512()
  gen_inv_gfni512()
  gen_pclmulqdq()

  term()

if __name__ == '__main__':
  main()
