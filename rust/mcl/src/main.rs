// env RUSTFLAGS="-L ../../../mcl/lib" cargo run

fn main() {
	println!("{}", mcl::hello());
	println!("version={:04x}", mcl::get_version());
	let b = mcl::init(mcl::CurveType::BN254);
	if !b {
		println!("init err");
	}
}
