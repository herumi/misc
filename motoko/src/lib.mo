/**
 * Module      : fp.mo
 * Description : finite field
 * Copyright   : 2022 Mitsunari Shigeo
 * License     : Apache 2.0 with LLVM Exception
 * Maintainer  : herumi <herumi@nifty.com>
 * Stability   : Stable
 */

import Int "mo:base/Int";

module {
  private let p_ : Nat = 65537;
  public class Fp() {
    private var v_ : Nat = 0;
    public func get(): Nat { v_ };
    public func set(v : Nat) {
      v_ := v % p_;
    };
    // set v without modulo
    public func set_nomod(v : Nat) {
      v_ := v;
    };
    public func is_zero() : Bool {
      v_ == 0
    };
  };
  // return (gcd, x, y) such that gcd = a * x + b * y
  public func ext_gcd(a:Int, b:Int) :(Int, Int, Int) {
    if (a == 0) return (b, 0, 1);
    let q = b / a;
    let r = b % a;
    let (gcd, x, y) = ext_gcd(r, a);
    (gcd, y - q * x, x)
  };
  // return rev such that x * rev mod p = 1
  public func inv_mod(x:Int, p:Int) : Int {
    let (gcd, rev, dummy) = ext_gcd(x, p);
    if (gcd != 1) return 0;
    if (rev < p) return rev + p;
    rev;
  };
  public func add(x : Fp, y : Fp) : Fp {
    var v = x.get() + y.get();
    if (v >= p_) v -= p_;
    let ret = Fp();
    ret.set_nomod(v);
    ret
  };
  public func sub(x : Fp, y : Fp) : Fp {
    var v:Nat = 0;
    if (x.get() >= y.get()) {
      v := x.get() - y.get();
    } else {
      v := x.get() + p_ - y.get();
    };
    let ret = Fp();
    ret.set_nomod(v);
    ret
  };
  public func mul(x : Fp, y : Fp) : Fp {
    let ret = Fp();
    ret.set(x.get() * y.get());
    ret
  };
  public func inv(x : Fp) : ?Fp {
    let v = inv_mod(x.get(), p_);
    if (v <= 0) return null;
    let ret = Fp();
    ret.set_nomod(Int.abs(v));
    ?ret
  };
  public func neg(x : Fp) : Fp {
    if (x.get() == 0) return Fp();
    let v = Fp();
    v.set_nomod(p_ - x.get());
    v
  };
};
