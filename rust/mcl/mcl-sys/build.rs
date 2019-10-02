use std::env;
use std::path::PathBuf;

fn main() {
	println!("cargo:rustc-link-lib=mcl");
	let bindings = bindgen::Builder::default()
		.header("wrapper.h")
		// don't insert space between -I and path
		.clang_arg("-I../../../../mcl/include/")
		.generate()
		.expect("Unable to generate bindings");

	let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
	bindings
		.write_to_file(out_path.join("bindings.rs"))
		.expect("Couldn't write bindings");
}
