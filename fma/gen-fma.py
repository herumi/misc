from s_xbyak import *

def gen_func(n):
  with FuncProc(f'func{n}'):
    with StackFrame(1, vNum=n+1, vType=T_ZMM) as sf:
      c = sf.p[0]
      lp = Label()
      for i in range(n):
        vxorps(Zmm(i), Zmm(i), Zmm(i))
      align(32)
      L(lp)
      for i in range(n):
        vfmadd231ps(Zmm(i), Zmm(i), Zmm(i))
      sub(c, 1)
      jnz(lp)

def main():
  parser = getDefaultParser()
  param = parser.parse_args()

  N=16

  init(param)
  segment('text')

  for i in range(1, N+1):
    gen_func(i)

  term()

if __name__ == '__main__':
  main()
