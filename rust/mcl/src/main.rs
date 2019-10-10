// env RUSTFLAGS="-L ../../../mcl/lib" cargo run
use mcl::*;

fn main() {
    println!("mcl version={:04x}", get_version());
    let b = init(CurveType::BN254);
    if !b {
        println!("init err");
    }
    let mut x = Fr::zero();
    println!("x={}", x.get_str(10));
    x.set_int(123456);
    println!("x={}", x.get_str(10));
    x.set_int(0xfff);
    println!("x={}", x.get_str(16));
    x.clear();
    println!("x={}", x.get_str(10));
    x.set_str("0x123", 0);
    println!("x={}", x.get_str(16));
    let buf = x.serialize();
    println!("serialize={:x?}", buf); // put hex byte
    let mut y = Fr::zero();
    if y.deserialize(&buf) {
        println!("y={}", y.get_str(16));
    } else {
        println!("err deserialize");
    }
    if x != y {
        println!("ng");
    }
    x.set_int(1);
    if x == y {
        println!("ng");
    }
    if !x.is_one() {
        println!("ng");
    }
    x.set_int(123);
    y.set_int(567);
    let mut z = Fr::uninit();
    Fr::add(&mut z, &x, &y);

    let x1 = Fr::from_str("1234", 10).unwrap();
    println!("x1={}", x1.get_str(10));

    println!("z={}", z.get_str(10));
    println!("x={}", x.get_str(10));
    println!("y={}", y.get_str(10));
    let mut P = G1::uninit();
    let mut Q = G2::uninit();
    let mut e = GT::uninit();
    P.set_hash_of("abc".as_bytes());
    Q.set_hash_of("abc".as_bytes());
    pairing(&mut e, &P, &Q);
}
