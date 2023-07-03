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
      mov(eax, 0x1)
      vmovd(xmm1, eax)
      vpshufd(ymm1, ymm1, 0)
      vgf2p8affineinvqb(ymm0, ymm1, ptr(px), 0)
      vmovups(ptr(py), ymm0)

def main():

  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')

  gen_mul_gfni()
  gen_inv_gfni()

  term()

if __name__ == '__main__':
  main()
