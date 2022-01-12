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
  // Ec/Fp : y^2 = x^3 + ax + b
  // (gx, gy) in Ec
  // #Ec = r
  private let p_ : Nat = 65537;
  private let a_ = 3;
  private let b_ = 5;
  private let r_ : Nat = 0;
  private let gx_ : Nat = 0;
  private let gy_ : Nat = 0;

  public func get_p() : Nat {
    p_
  };

  // return (gcd, x, y) such that gcd = a * x + b * y
  public func ext_gcd(a:Int, b:Int) : (Int, Int, Int) {
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
    assert(gcd == 1);
    var v = rev;
    if (rev < 0) v := rev + p;
    assert(0 <= v and v < p);
    Int.abs(v)
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
  // return y^2 == x^3 + ax + b
  public func is_valid(x:Fp, y:Fp) : Bool {
    y.mul(y).get() == (x.mul(x).add(newFp_nomod(a_))).mul(x).add(newFp_nomod(b_)).get()
  };
  public class Ec() {
    private var x_  = 0;
    private var y_  = 0;
    private var isZero_ : Bool = true;
    public func get() : (Nat, Nat) { (x_, y_) };
    public func is_zero() : Bool { isZero_ };
    public func is_valid() : Bool {
      if (isZero_) return true;
//      return is_valid(x_, y_);
      false
    };
  };
};
