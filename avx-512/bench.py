from s_xbyak import *

N=8

def gen_select(mode):
  align(32)
  with FuncProc(f'select{mode}'):
    with StackFrame(3, vNum=1, vType=T_ZMM) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      vmovups(zm0, ptr(px))
      vmovups(zm1, ptr(py))
      vpcmpeqq(k1, zm0, zm1)
      if mode == 0:
        # faster
        for i in range(N):
          vmovdqu64(zm0, ptr(px + 64*i))
          vmovdqu64(zm0|k1, ptr(py + 64*i))
          vmovdqu64(ptr(pz + 64*i), zm0)
      else:
        vpternlogq(zm2|k1|T_z, zm2, zm2, 0xff)
        for i in range(N):
          vmovdqu64(zm0, ptr(px + 64*i))
          vpandq(zm0, zm0, zm2)
          vpandq(zm1, zm2, ptr(py + 64*i))
          vporq(zm0, zm0, zm1)
          vmovdqu64(ptr(px + 64*i), zm0)

      vzeroupper()
      ret()

def gen_misc():
  align(32)
  with FuncProc(f'misc'):
    with StackFrame(3, vNum=1, vType=T_ZMM) as sf:
      pz = sf.p[0]
      px = sf.p[1]
      py = sf.p[2]
      vmovups(zm0, ptr(px))
      vmovups(zm1, ptr(py))
      vpcmpeqq(k1, zm0, zm1)
      vpternlogq(zm0|k1|T_z, zm0, zm0, 0xff) # zm0 = k1 ? -1 : 0
      vmovups(ptr(sf.p[0]), zm0)
      ret()

def main():
  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('text')
  gen_select(0)
  gen_select(1)
  gen_misc()
  term()

if __name__ == '__main__':
  main()
