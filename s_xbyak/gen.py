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

def gen_vpalignr():
  align(32)
  with FuncProc(f'gen_vpalignr'):
    # void load(uint8_t *py, const uint8_t *px);
    with StackFrame(2, vNum=2, vType=T_YMM) as sf:
      py = sf.p[0]
      px = sf.p[1]
      vxorps(ym0, ym0, ym0)
      vmovupd(ym1, ptr(px))
      # left-shift of ym0 with 1 byte
      vpalignr(ym1, ym1, ym0, 15)
      vmovupd(ptr(py), ym1)

# set y by px[0:n] as float
# return the label for continued execution
# yt, t : tmp regs
def loadf(y, px, n, N, yt, t):
  contiL = Label()
  # prepare jmp table
  loadL = []
  for i in range(N):
    loadL.append(Label())
  loadTopL = Label()
  vxorps(y, y, y)
  lea(t, ptr(rip + loadTopL))
  jmp(ptr(t + n * 8))
  segment('data')
  align(32)
  L(loadTopL)
  dq_(0)
  for i in range(N):
    dq_(loadL[i])
  segment('text')

  for i in range(N):
    L(loadL[i])
    if i == 0:
      vmovss(xm0, ptr(px))
      jmp(contiL)
      continue
    if i == 1:
      vmovq(xm0, ptr(px))
      jmp(contiL)
      continue
    if i == 2:
      vmovq(xm0, ptr(px))
      vmovss(xm1, ptr(px+4*2))
      vpunpcklqdq(ym0, ym0, ym1)
      jmp(contiL)
      continue
    if i == 3:
      vmovups(xm0, ptr(px))
      jmp(contiL)
      continue
    if i == 4:
      vmovups(xm0, ptr(px))
      vmovss(xm1, ptr(px+4*4))
      vinserti128(ym0, ym0, xm1, 1)
      jmp(contiL)
      continue
    else:
      for j in range((i+1)%4):
        if j == 0:
          vmovss(xm0, ptr(px + (i-j)*4))
        else:
          vpshufd(ym0, ym0, 0b10010011) # ym0 <<= 32 (assume ym0[3] == 0)
          vmovss(xm1, ptr(px + (i-j)*4))
          vorps(ym0, ym0, ym1)
      if i >= 3:
        vmovups(xm1, ptr(px))
        vinserti128(ym0, ym1, xm0, 1)
    jmp(contiL)

  return contiL

def gen_loadf():
  align(32)
  with FuncProc(f'loadf'):
    # void load(float *py, const float *px, size_t n);
    with StackFrame(3, vNum=3, vType=T_YMM) as sf:
      py = sf.p[0]
      px = sf.p[1]
      n = sf.p[2]
      vxorps(ym0, ym0, ym0)
      vxorps(ym1, ym1, ym1)
      loadDoneL = Label()
      L_N = 8

      contiL = loadf(ym0, px, n, L_N, ym1, rax)

      L(contiL)

      vaddps(ym0, ym0, ym0)
      vmovups(ptr(py), ym0)



def main():
  parser = getDefaultParser()
  param = parser.parse_args()

  init(param)
  segment('data')
  align(32)
#  makeLabel('shiftPtn')
#  dq_(0x0706050480808080)
#  dq_(0x0f0e0d0c0b0a0908)
#  dq_(0x13121110)
#  dq_(0x1b1a191817161514)
#  dq_(0x232221201f1e1d1c)
  segment('text')
  gen_fma()
  gen_mov()
  gen_vpalignr()
  gen_loadf()

  term()

if __name__ == '__main__':
  main()
