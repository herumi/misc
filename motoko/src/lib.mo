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

  // return (gcd, x, y) such that gcd = a * x + b * y
  public func ext_gcd(a:Int, b:Int) :(Int, Int, Int) {
    if (a == 0) return (b, 0, 1);
    let q = b / a;
    let r = b % a;
    let (gcd, x, y) = ext_gcd(r, a);
    (gcd, y - q * x, x)
  };
  public func add_mod(x:Nat, y:Nat, p:Nat) : Nat {
    let v = x + y;
    if (v < p) return v;
    return v - p;
  };
  public func sub_mod(x:Nat, y:Nat, p:Nat) : Nat {
    if (x >= y) return x - y;
    x + p - y;
  };
  public func mul_mod(x:Nat, y:Nat, p:Nat) : Nat {
    (x * y) % p;
  };
  // return rev such that x * rev mod p = 1 if success else 0
  public func inv_mod(x:Nat, p:Nat) : Nat {
    let (gcd, rev, dummy) = ext_gcd(x, p);
    if (gcd != 1) return 0;
    var v = rev;
    if (rev < 0) v := rev + p;
    // use assert?
    if (0 <= v and v < p) return Int.abs(v);
    0;
  };
  public func neg_mod(x:Nat, p:Nat) : Nat {
    if (x == 0) return 0;
    p - x;
  };

  public class Fp() {
    private var v_ : Nat = 0;
    public func get(): Nat { v_ };
    public func set(v:Nat) {
      v_ := v % p_;
    };
    // set v without modulo
    public func set_nomod(v:Nat) {
      v_ := v;
    };
    public func is_zero() : Bool {
      v_ == 0
    };
    public func add(rhs:Fp) : Fp {
      let ret = Fp();
      ret.set_nomod(add_mod(v_, rhs.get(), p_));
      ret
    };
    public func sub(rhs:Fp) : Fp {
      let ret = Fp();
      ret.set_nomod(sub_mod(v_, rhs.get(), p_));
      ret
    };
    public func mul(rhs:Fp) : Fp {
      let ret = Fp();
      ret.set_nomod(mul_mod(v_, rhs.get(), p_));
      ret
    };
    public func inv() : Fp {
      let ret = Fp();
      ret.set_nomod(inv_mod(v_, p_));
      ret
    };
    public func div(rhs:Fp) : Fp {
      mul(rhs.inv())
    };
    public func neg() : Fp {
      let ret = Fp();
      ret.set_nomod(neg_mod(v_, p_));
      ret
    };
  };
  public func newFp(x:Nat) : Fp {
    let ret = Fp();
    ret.set(x);
    ret
  };
  public func newFp_nomod(x:Nat) : Fp {
    let ret = Fp();
    ret.set_nomod(x);
    ret
  };
};
