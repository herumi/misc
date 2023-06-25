from s_xbyak import *

DATA='data'

def gen_func(op, n):
  with FuncProc(f'func{n}'):
    with StackFrame(1, vNum=n+1, vType=T_ZMM) as sf:
      c = sf.p[0]
      lp = Label()
      for i in range(n):
        vxorps(Zmm(i), Zmm(i), Zmm(i))
      xor_(eax, eax)
      kmovd(k1, eax)
      lea(rax, ptr(rip+DATA))
      align(32)
      L(lp)
      if op == 'imm-bcst':
        mov(rax, 0x123)
        vpbroadcastd(Zmm(n), eax)
      for i in range(n):
        if op == 'none':
          pass
        elif op == 'fma':
          vfmadd231ps(Zmm(i), Zmm(i), Zmm(i))
        elif op == 'mem':
          vfmadd231ps(Zmm(i), Zmm(i), ptr_b(rax+i*4))
        elif op == 'mem-rip':
          vfmadd231ps(Zmm(i), Zmm(i), ptr_b(rip+DATA))
        elif op == 'imm-bcst':
          vfmadd231ps(Zmm(i), Zmm(i), Zmm(n))
        elif op == 'add':
          vaddps(Zmm(i), Zmm(i), Zmm(i))
        elif op == 'mul':
          vmulps(Zmm(i), Zmm(i), Zmm(i))
        elif op == 'xor':
          vxorps(Zmm(i), Zmm(i), Zmm(i))
        elif op == 'sqrt':
          vrsqrt14ps(Zmm(i), Zmm(i))
        elif op == 'and':
          vandps(Zmm(i), Zmm(i), Zmm(i))
        elif op == 'red':
          vreduceps(Zmm(i), Zmm(i), 0)
        elif op == 'rnd':
          vrndscaleps(Zmm(i), Zmm(i), 0)
        elif op == 'gather':
          vpgatherdd(Zmm(i+1)|k1, ptr(rax+Zmm(i)*4))
        else:
          raise Exception('bad op', op)
      sub(c, 1)
      jnz(lp)

def gen_header(n):
  print('extern "C" {')
  for i in range(1, n+1):
    print(f'void func{i}(int n);')
  print('}')
  print('template<int N>void funcN(int n);')
  for i in range(1, n+1):
    print(f'template<>void funcN<{i}>(int n) {{ func{i}(n); }}')
  print('template<int n>void bench();')
  print('''void benchAll(int n)
{''')
  for i in range(1, n+1):
    print(f'\t\tif (n == 0 || n == {i}) bench<{i}>();')
  print('}')

def main():
  parser = getDefaultParser()
  parser.add_argument('-header', '--header', help='generate a header', action='store_true')
  parser.add_argument('-op', '--op', help='operand', type=str, default='fma')
  param = parser.parse_args()

  N=16

  if param.header:
    gen_header(N)
    return

  init(param)
  segment('data')
  makeLabel(DATA)
  for i in range(32):
    dd_(i)
  segment('text')

  for i in range(1, N+1):
    gen_func(param.op, i)

  term()

if __name__ == '__main__':
  main()
