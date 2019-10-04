use std::os::raw::{c_int};

#[link(name = "mcl", kind = "static")]
#[link(name = "mclbn384_256", kind = "static")]
#[link(name = "gmp")]
#[link(name = "stdc++")]
#[link(name = "crypto")]

extern "C" {
	fn mclBn_getVersion() -> c_int;
	fn mclBn_init(curve : c_int, compiledTimeVar : c_int) -> c_int;
	fn mclBnFr_clear(x :*mut u64);
	fn mclBnFr_setInt32(y :*mut u64, x :i32);
	fn mclBnFr_getStr(buf :*mut u8, maxBufSize: usize, x:*const u64, ioMode:c_int) -> usize;
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

#[allow(dead_code)]
#[repr(C)]
pub struct Fp {
	d:[u64; MCLBN_FP_UNIT_SIZE]
}

#[allow(dead_code)]
#[repr(C)]
pub struct Fp2 {
	d:[Fr; 2]
}

#[repr(C)]
#[derive(Default)]
pub struct Fr {
	d:[u64; MCLBN_FR_UNIT_SIZE]
}

#[allow(dead_code)]
#[repr(C)]
pub struct G1 {
	x:Fp,
	y:Fp,
	z:Fp
}

#[allow(dead_code)]
#[repr(C)]
pub struct G2 {
	x:Fp2,
	y:Fp2,
	z:Fp2
}

#[allow(dead_code)]
#[repr(C)]
pub struct GT {
	d:[Fp; 12]
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

impl Fr {
	pub fn zero() -> Fr {
		Default::default()
	}
	pub fn clear(&mut self) {
		unsafe {
			mclBnFr_clear(self.d.as_mut_ptr());
		}
	}
	pub fn set_int(&mut self, x :i32) {
		unsafe {
			mclBnFr_setInt32(self.d.as_mut_ptr(), x);
		}
	}
	pub fn to_string(&self) -> String {
		let mut d:[u8; 1024] = unsafe { std::mem::uninitialized() };
		let n:usize;
		unsafe {
			n = mclBnFr_getStr(d.as_mut_ptr(), d.len(), self.d.as_ptr(), 0);
		}
		if n == 0 {
			panic!("mclBnFr_getStr");
		}
		d[0..n].iter().map(|&s| s as char).collect::<String>()
	}
}
