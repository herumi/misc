// env RUSTFLAGS="-L ../../../mcl/lib" cargo run

fn main() {
	println!("mcl version={:04x}", mcl::get_version());
	let b = mcl::init(mcl::CurveType::BN254);
	if !b {
		println!("init err");
	}
	let mut x:mcl::Fr = unsafe { std::mem::uninitialized() };
	x.clear();
	println!("x={}", x.to_string());
}
