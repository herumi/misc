from s_xbyak import *

def gen_asm():
  with FuncProc('gf256_mul_gfni'):
    with StackFrame(3, vNum=1, vType=T_YMM) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      vmovups(ymm0, ptr(px))
      vgf2p8mulb(ymm0, ymm0, ptr(py))
      vmovups(ptr(pz), ymm0)

def main():

  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')

  gen_asm()

  term()

if __name__ == '__main__':
  main()
