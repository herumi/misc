// env RUSTFLAGS="-L ../../../mcl/lib" cargo run
use std::os::raw::{c_int};
extern crate mcl;

#[link(name = "mcl", kind = "static")]
#[link(name = "mclbn384_256", kind = "static")]
#[link(name = "gmp")]
#[link(name = "stdc++")]
#[link(name = "crypto")]

extern "C" {
	fn mclBn_getVersion() -> c_int;
}
fn main() {
	println!("{}", mcl::hello());
	unsafe {
		println!("version={:04x}", mclBn_getVersion());
	}
}
