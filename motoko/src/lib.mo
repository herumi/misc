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
import Nat "mo:base/Nat";
import Nat8 "mo:base/Nat8";
import Nat32 "mo:base/Nat32";
import Nat64 "mo:base/Nat64";
import Blob "mo:base/Blob";
import IterExt "mo:iterext";

module {

  public func sum(x : Nat) : Nat {
	2:Nat**x;
  };
  private let p : Nat = 65537;
  public class Fp() {
    private var v_ : Nat = 0;
    public func get(): Nat { v_ };
    public func set(v : Nat) {
      v_ := v;
    };
  };
};
