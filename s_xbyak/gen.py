from s_xbyak import *

def gen():
  with FuncProc(f'func'):
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
  gen()

  term()

if __name__ == '__main__':
  main()
