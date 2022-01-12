import Int "mo:base/Int";
import M "../src";

let p = M.get_prime();

func cstr_test() {
  assert(M.Fp().get() == 0);
  var x = M.Fp();
  assert(x.is_zero());
  var v1 = p + 123;
  x.set_nomod(v1);
  assert(not x.is_zero());
  assert(x.get() == v1);
  x.set(v1);
  assert(x.get() == v1 % p);
};

func arith_test() {
  let m1 = 50000;
  let m2 = 60000;
  var x1 = M.newFp(m1);
  var x2 = M.newFp(m2);
  assert(x1.add(x2).get() == (m1 + m2) % p);
  assert(x1.sub(x2).get() == (m1 + p - m2) % p);
  assert(x2.sub(x1).get() == (m2 - m1) % p);
  assert(M.Fp().neg().is_zero());
  var x3 = M.newFp(m1).neg();
  assert(x3.get() == p - m1);
  x1.set(m1);
  x2.set(m2);
  x3 := x1.mul(x2);
  assert(x3.get() == (m1 * m2) % p);
};

func gcd_test() {
  let (gcd1, gcd2, gcd3) = M.ext_gcd(100, 37);
  assert(gcd1 == 1);
  assert(gcd2 == 10);
  assert(gcd3 == -27);
  let (a, b, c) = M.ext_gcd(0, 37);
  assert(a == 37);
  assert(b == 0);
  assert(c == 1);
};

func inv_test() {
  let inv123 = M.inv_mod(123, 65537);
  assert(inv123 == 14919);
  let x2 = M.newFp(123).inv();
  assert(x2.get() == 14919);
  var i = 1;
  while (i < 20) {
    let x1 = M.newFp(i);
    assert(x1.mul(x1.inv()).get() == 1);
    assert(x2.div(x1).mul(x1).get() == x2.get());
    i += 1;
  };
};

cstr_test();
arith_test();
gcd_test();
inv_test();
