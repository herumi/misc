// env RUSTFLAGS="-L ../../../mcl/lib" cargo run

fn main() {
	println!("mcl version={:04x}", mcl::get_version());
	let b = mcl::init(mcl::CurveType::BN254);
	if !b {
		println!("init err");
	}
	let mut x = mcl::Fr::zero();
	println!("x={}", x.to_string());
	x.set_int(123456);
	println!("x={}", x.to_string());
	x.clear();
	println!("x={}", x.to_string());
}
