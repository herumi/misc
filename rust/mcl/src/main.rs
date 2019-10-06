// env RUSTFLAGS="-L ../../../mcl/lib" cargo run

fn main() {
	println!("mcl version={:04x}", mcl::get_version());
	let b = mcl::init(mcl::CurveType::BN254);
	if !b {
		println!("init err");
	}
	let mut x = mcl::Fr::zero();
	println!("x={}", x.get_str(10));
	x.set_int(123456);
	println!("x={}", x.get_str(10));
	x.set_int(0xfff);
	println!("x={}", x.get_str(16));
	x.clear();
	println!("x={}", x.get_str(10));
	x.set_str("0x123", 0);
	println!("x={}", x.get_str(16));
	println!("serialize={:?}", x.serialize());
}
