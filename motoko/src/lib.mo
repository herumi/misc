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
    let ret = Fp();
    if (x.get() < y.get()) {
      ret.set_nomod(x.get() + p_ - y.get());
    } else {
      ret.set_nomod(x.get() - y.get());
    };
    ret;
  };
};
