import Blob "mo:base/Blob";
import M "../src";

// empty string

let b = Blob.fromArray([1:Nat8,2]);
let h1 = Blob.fromArray([3: Nat8, 4]);

assert(b == b);
assert(M.sum(255) == 57896044618658097711785492504343953926634992332820282019728792003956564819968);

assert(M.Fp().get() == 0);

let x = M.Fp();
x.set(5);
assert(x.get() == 5);

