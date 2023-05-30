import sys
sys.path.append('../../s_xbyak')
from s_xbyak import *

def gen_mov():
  align(32)
  with FuncProc(f'gen_mov'):
    with StackFrame(2, vNum=1, vType=T_ZMM) as sf:
      px = sf.p[0]
      py = sf.p[1]
      vpbroadcastd(zm0, ptr_b(py))
      vmovups(ptr(px), zm0)

def gen_fma():
  align(32)
  with FuncProc(f'gen_fma'):
    with StackFrame(3, vNum=3, vType=T_XMM) as sf:
      px = sf.p[0]
      py = sf.p[1]
      pz = sf.p[2]
      vmovss(xm0, ptr(px))
      vmovss(xm1, ptr(py))
      vmovss(xm2, ptr(pz))
      vfmadd213ss(xm0, xm1, xm2)
      vmovss(ptr(px), xm0)

def main():
  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')
  gen_fma()
  gen_mov()

  term()

if __name__ == '__main__':
  main()
