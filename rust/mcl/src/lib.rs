use std::os::raw::{c_int};
use std::mem::{MaybeUninit};

#[link(name = "mcl", kind = "static")]
#[link(name = "mclbn384_256", kind = "static")]
#[link(name = "gmp")]
#[link(name = "stdc++")]
#[link(name = "crypto")]

#[allow(non_snake_case)]
extern "C" {
	fn mclBn_getVersion() -> c_int;
	fn mclBn_getFrByteSize() -> c_int;
	fn mclBn_getFpByteSize() -> c_int;
	fn mclBn_init(curve : c_int, compiledTimeVar : c_int) -> c_int;
	fn mclBnFr_setInt32(x :*mut u64, v :i32);
	fn mclBnFr_setStr(x :*mut u64, buf: *const u8, bufSize: usize, ioMode:c_int) -> c_int;
	fn mclBnFr_getStr(buf :*mut u8, maxBufSize: usize, x:*const u64, ioMode:c_int) -> usize;
	fn mclBnFr_serialize(buf :*mut u8, maxBufSize: usize, x:*const u64) -> usize;
	fn mclBnFr_deserialize(x:*mut u64, buf:*const u8, bufSize: usize) -> usize;
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
		*self = Fr::zero()
	}
	pub fn set_int(&mut self, x :i32) {
		unsafe {
			mclBnFr_setInt32(self.d.as_mut_ptr(), x);
		}
	}
	pub fn set_str(&mut self, s:&str, base:i32) -> bool {
		unsafe {
			mclBnFr_setStr(self.d.as_mut_ptr(), s.as_ptr(), s.len(), base as c_int) == 0
		}
	}
	pub fn get_str(&self, io_mode:i32) -> String {
		let mut d:[u8; 1024] = unsafe { MaybeUninit::uninit().assume_init() };
		let n:usize;
		unsafe {
			n = mclBnFr_getStr(d.as_mut_ptr(), d.len(), self.d.as_ptr(), io_mode as c_int);
		}
		if n == 0 {
			panic!("mclBnFr_getStr");
		}
		d[0..n].iter().map(|&s| s as char).collect::<String>()
	}
	pub fn serialize(&self) -> Vec<u8> {
		let size = unsafe { mclBn_getFrByteSize() } as usize;
		let mut buf:Vec<u8> = Vec::with_capacity(size);
		let n:usize;
		unsafe {
			n = mclBnFr_serialize(buf.as_mut_ptr(), size, self.d.as_ptr());
		}
		if n == 0 {
			panic!("serialize");
		}
		unsafe { buf.set_len(n); }
		buf
	}
	pub fn deserialize(&mut self, buf:&[u8]) -> bool {
		unsafe {
			mclBnFr_deserialize(self.d.as_mut_ptr(), buf.as_ptr(), buf.len()) > 0
		}
	}
}
