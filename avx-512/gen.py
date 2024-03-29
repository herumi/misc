from s_xbyak import *

def gen_getmant_sub(mode):
  align(32)
  with FuncProc(f'getmant{mode}'):
    with StackFrame(2, vNum=1, vType=T_ZMM) as sf:
      px = sf.p[0]
      py = sf.p[1]
      vmovss(xm0, ptr(py))
      vgetmantss(xm0, xm0, xm0, mode)
      vmovss(ptr(px), xm0)

def gen_getmant():
  for i in range(4):
    gen_getmant_sub(i)

def gen_vreduceps():
  align(32)
  with FuncProc(f'vreduceps'):
    with StackFrame(2, vNum=2, vType=T_ZMM) as sf:
      px = sf.p[0]
      py = sf.p[1]
      vmovss(xm0, ptr(py))
      vrndscaleps(xm1, xm0, 0)
      vmovss(ptr(px), xm1)
      vreduceps(xm1, xm0, 0)
      vmovss(ptr(px+4), xm1)

def main():
  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')
  gen_getmant()
  gen_vreduceps()
  term()

if __name__ == '__main__':
  main()
