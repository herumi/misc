from s_xbyak import *

def gen_fma(n):
  with FuncProc(f'fma{n}'):
    with StackFrame(1, vNum=n, vType=T_ZMM) as sf:
      c = sf.p[0]
      lp = Label()
      for i in range(n):
        vxorps(Zmm(i), Zmm(i), Zmm(i))
#      align(16):
      L(lp)
      for i in range(n):
        vfmadd231ps(Zmm(i), Zmm(i), Zmm(i))
      sub(c, 1)
      jnz(lp)

def gen_header(n):
  print('extern "C" {')
  for i in range(1, n+1):
    print(f'void fma{i}(int n);')
  print('}')
  print('template<int N>void fmaN(int n);')
  for i in range(1, n+1):
    print(f'template<>void fmaN<{i}>(int n) {{ fma{i}(n); }}')


def main():
  parser = getDefaultParser()
  parser.add_argument('-header', '--header', help='generate a header', action='store_true')
  param = parser.parse_args()

  N=10

  if param.header:
    gen_header(N)
    return

  init(param)
  segment('text')

  for i in range(1, N+1):
    gen_fma(i)

  term()

if __name__ == '__main__':
  main()
