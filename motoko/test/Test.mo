import Blob "mo:base/Blob";
import M "../src";

assert(M.Fp().get() == 0);

var x = M.Fp();
assert(x.is_zero());
let p = 65537;
var v1 = p + 123;

x.set_nomod(v1);
assert(not x.is_zero());
assert(x.get() == v1);
x.set(v1);
assert(x.get() == v1 % p);

let m1 = 50000;
let m2 = 60000;

var x1 = M.Fp();
var x2 = M.Fp();
x1.set(m1);
x2.set(m2);
var x3 = M.add(x1, x2);
assert(x3.get() == (m1 + m2) % p);

x3 := M.sub(x1, x2);
assert(x3.get() == (m1 + p - m2) % p);

x3 := M.sub(x2, x1);
assert(x3.get() == (m2 - m1) % p);

x3 := M.neg(M.Fp());
assert(x3.is_zero());
x3.set(m1);
x3 := M.neg(x3);
assert(x3.get() == p - m1);
