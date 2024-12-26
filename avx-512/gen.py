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

def gen_vpermps():
  align(32)
  with FuncProc(f'vperm1'):
    with StackFrame(3, vNum=2, vType=T_ZMM) as sf:
      py = sf.p[0]
      px = sf.p[1]
      pidx = sf.p[2]
      vmovups(zm0, ptr(px))
      vmovups(zm1, ptr(pidx))
      vpermps(zm0, zm1, zm0)
      vmovups(ptr(py), zm0)

  align(32)
  with FuncProc(f'vperm2'):
    with StackFrame(3, vNum=3, vType=T_YMM) as sf:
      py = sf.p[0]
      px = sf.p[1]
      pidx = sf.p[2]
      vmovups(ym0, ptr(px))
      vmovups(ym1, ptr(px+32))
      vmovups(ym2, ptr(pidx))
      vpslld(ym3, ym2, 31-3)
      vblendvps(ym0, ym1, ym0, ym3)
      vpermps(ym0, ym2, ym0)
      vmovups(ptr(py), ym0)

def main():
  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')
  gen_getmant()
  gen_vreduceps()
  gen_vpermps()
  term()

if __name__ == '__main__':
  main()
