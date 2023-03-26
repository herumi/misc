import sys
sys.path.append('../../mcl-ff')
from s_xbyak_llvm import *

def main():
  r = Int(32)
  n = Int(32)
  zero = Imm(0, 32)
  with Function('sum', r, n):
    entry = Label()
    lp = Label()
    body = Label()
    end = Label()

    L(entry)
    br(lp)
    L(lp)
    phi_i = i = phi(zero, entry)
    s = phi(zero, entry)
    cond = icmp(ult, i, n)
    br(cond, body, end)

    L(body)
    s2 = add(s, i)
    i2 = add(i, 1)
    phi_i.link(i2, body)
    s.link(s2, body)
    br(lp)

    L(end)
    r = phi((zero, entry), (s2, lp))
    ret(r)

  term()

if __name__ == '__main__':
  main()
