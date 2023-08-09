import sys
sys.path.append('../../mcl-ff')
from s_xbyak_llvm import *

BIT=64
def gen_inv(N):
  py = IntPtr(BIT)
  px = IntPtr(BIT)
  pm = IntPtr(BIT)
  k = Imm(0, 32)
  with Function(f'invModPre{N}', k, py, px, pm):
    entry = Label()
    L(entry)
    ZERO = Imm(0, BIT*N)
    ZERO32 = Imm(0, 32)
    ONE = Imm(1, BIT*N)
    m = loadN(pm, N)
    x = loadN(px, N)
    u = m
    v = x
    r = ZERO
    s = ONE
    k = ZERO32

    lp = Label()
    conti = Label()
    even_u = Label()
    even_v = Label()
    odd_u = Label()
    odd_v = Label()
    u_gt_v = Label()
    u_le_v = Label()
    exit_lp = Label()
    br(icmp('eq', x, 0), exit_lp, lp)

    L(lp)
    phi_k = k = phi(ZERO32, entry)
    phi_s = s = phi(ONE, entry)
    phi_r = r = phi(ZERO, entry)
    phi_v = v = phi(x, entry)
    phi_u = u = phi(m, entry)
    br(trunc(u, 1), odd_u, even_u)

    L(even_u)
    u1 = ashr(u, 1)
    s1 = shl(s, 1)
    br(conti)

    L(odd_u)
    br(trunc(v, 1), odd_v, even_v)

    L(even_v)
    v2 = ashr(v, 1)
    r2 = shl(r, 1)
    br(conti)

    L(odd_v)
    br(icmp('ugt', u, v), u_gt_v, u_le_v)

    L(u_gt_v)
    u3 = ashr(sub(u, v), 1)
    r3 = add(r, s)
    s3 = shl(s, 1)
    br(conti)

    L(u_le_v)
    v4 = ashr(sub(v, u), 1)
    s4 = add(s, r)
    r4 = shl(r, 1)
    br(conti)

    L(conti)
    u = phi((u1, even_u), (u,  even_v), (u3, u_gt_v), (u,  u_le_v))
    v = phi((v,  even_u), (v2, even_v), (v,  u_gt_v), (v4, u_le_v))
    r = phi((r,  even_u), (r2, even_v), (r3, u_gt_v), (r4, u_le_v))
    s = phi((s1, even_u), (s,  even_v), (s3, u_gt_v), (s4, u_le_v))

    k = add(k, 1)

    phi_k.link(k, conti)
    phi_s.link(s, conti)
    phi_r.link(r, conti)
    phi_v.link(v, conti)
    phi_u.link(u, conti)
    br(icmp('eq', v, ZERO), exit_lp, lp)

    L(exit_lp)
    r = phi((ZERO, entry), (r, conti))
    k = phi((ZERO32, entry), (k, conti))
    r = select(icmp('ugt', r, m), sub(r, m), r)
    y = sub(m, r)
    storeN(y, py)
    ret(k)
    

init()
gen_inv(3)
term()
