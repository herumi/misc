use std::os::raw::{c_int};

#[link(name = "mcl", kind = "static")]
#[link(name = "mclbn384_256", kind = "static")]
#[link(name = "gmp")]
#[link(name = "stdc++")]
#[link(name = "crypto")]
extern "C" {
	fn mclBn_getVersion() -> c_int;
	fn mclBn_init(curve : c_int, compiledTimeVar : c_int) -> c_int;
}

pub enum CurveType {
	BN254 = 0,
	BN381 = 1,
	SNARK = 4,
	BLS12_381 = 5,
}

const MCLBN_FP_UNIT_SIZE:usize = 6;
const MCLBN_FR_UNIT_SIZE:usize = 4;
const MCLBN_COMPILED_TIME_VAR:c_int = (MCLBN_FR_UNIT_SIZE as c_int * 10 + MCLBN_FP_UNIT_SIZE as c_int);

pub struct Fp {
	#[allow(dead_code)]
	d:[u64; MCLBN_FP_UNIT_SIZE]
}

pub struct Fr {
	#[allow(dead_code)]
	d:[u64; MCLBN_FR_UNIT_SIZE]
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}

pub fn hello() -> String {
	"abc".to_string()
}

pub fn get_version() -> u32 {
	unsafe {
		mclBn_getVersion() as u32
	}
}

pub fn init(curve:CurveType) -> bool {
	unsafe {
		let r = mclBn_init(curve as c_int, MCLBN_COMPILED_TIME_VAR);
		r as u32 == 0
	}
}
