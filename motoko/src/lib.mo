/**
 * Module      : fp.mo
 * Description : finite field
 * Copyright   : 2022 Mitsunari Shigeo
 * License     : Apache 2.0 with LLVM Exception
 * Maintainer  : herumi <herumi@nifty.com>
 * Stability   : Stable
 */

import Array "mo:base/Array";
import Buffer "mo:base/Buffer";
import Iter "mo:base/Iter";
import Blob "mo:base/Blob";
import IterExt "mo:iterext";

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
      return v_ == 0;
    };
  };
  public func add(x : Fp, y : Fp) : Fp {
    var v = x.get() + y.get();
    if (v >= p_) {
      v -= p_;
    };
    let ret = Fp();
    ret.set_nomod(v);
    ret;
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
    ret;
  };
  public func mul(x : Fp, y : Fp) : Fp {
    let ret = Fp();
    ret.set((x.get() * y.get()) % p_);
    ret;
  };
  public func neg(x : Fp) : Fp {
    if (x.get() == 0) {
      Fp();
    } else {
      let v = Fp();
      v.set_nomod(p_ - x.get());
      v;
    };
  };
};
