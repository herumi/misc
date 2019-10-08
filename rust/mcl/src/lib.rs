use std::mem::MaybeUninit;
use std::os::raw::c_int;

#[link(name = "mcl", kind = "static")]
#[link(name = "mclbn384_256", kind = "static")]
#[link(name = "gmp")]
#[link(name = "stdc++")]
#[link(name = "crypto")]
#[allow(non_snake_case)]
extern "C" {
    fn mclBn_getVersion() -> u32;
    fn mclBn_getFrByteSize() -> u32;
    fn mclBn_getFpByteSize() -> u32;
    fn mclBn_init(curve: c_int, compiledTimeVar: c_int) -> c_int;
    fn mclBnFr_setInt32(x: *mut u64, v: i32);
    fn mclBnFr_setStr(x: *mut u64, buf: *const u8, bufSize: usize, ioMode: c_int) -> c_int;
    fn mclBnFr_getStr(buf: *mut u8, maxBufSize: usize, x: *const u64, ioMode: c_int) -> usize;
    fn mclBnFr_serialize(buf: *mut u8, maxBufSize: usize, x: *const u64) -> usize;
    fn mclBnFr_deserialize(x: *mut u64, buf: *const u8, bufSize: usize) -> usize;
}

pub enum CurveType {
    BN254 = 0,
    BN381 = 1,
    SNARK = 4,
    BLS12_381 = 5,
}

const MCLBN_FP_UNIT_SIZE: usize = 6;
const MCLBN_FR_UNIT_SIZE: usize = 4;
const MCLBN_COMPILED_TIME_VAR: c_int =
    (MCLBN_FR_UNIT_SIZE as c_int * 10 + MCLBN_FP_UNIT_SIZE as c_int);

macro_rules! serialize_impl {
    ($t:ty, $size:expr, $serialize_fn:ident) => {
        impl $t {
            pub fn serialize(&self) -> Vec<u8> {
                let size = unsafe { $size } as usize;
                make_serialize(size, &self.d, $serialize_fn)
            }
        }
    };
}

#[allow(dead_code)]
#[repr(C)]
pub struct Fp {
    d: [u64; MCLBN_FP_UNIT_SIZE],
}

#[allow(dead_code)]
#[repr(C)]
pub struct Fp2 {
    d: [Fr; 2],
}

#[repr(C)]
#[derive(Default)]
pub struct Fr {
    d: [u64; MCLBN_FR_UNIT_SIZE],
}
serialize_impl![Fr, mclBn_getFrByteSize(), mclBnFr_serialize];

#[allow(dead_code)]
#[repr(C)]
pub struct G1 {
    x: Fp,
    y: Fp,
    z: Fp,
}

#[allow(dead_code)]
#[repr(C)]
pub struct G2 {
    x: Fp2,
    y: Fp2,
    z: Fp2,
}

#[allow(dead_code)]
#[repr(C)]
pub struct GT {
    d: [Fp; 12],
}

pub fn get_version() -> u32 {
    unsafe { mclBn_getVersion() }
}

pub fn init(curve: CurveType) -> bool {
    unsafe { mclBn_init(curve as c_int, MCLBN_COMPILED_TIME_VAR) == 0 }
}

fn make_serialize(
    size: usize,
    d: &[u64],
    serialize: unsafe extern "C" fn(buf: *mut u8, maxBufSize: usize, x: *const u64) -> usize,
) -> Vec<u8> {
    let mut buf: Vec<u8> = Vec::with_capacity(size);
    let n: usize;
    unsafe {
        n = serialize(buf.as_mut_ptr(), size, d.as_ptr());
    }
    if n == 0 {
        panic!("serialize");
    }
    unsafe {
        buf.set_len(n);
    }
    buf
}

impl Fr {
    pub fn zero() -> Fr {
        Default::default()
    }
    pub fn clear(&mut self) {
        *self = Fr::zero()
    }
    pub fn set_int(&mut self, x: i32) {
        unsafe {
            mclBnFr_setInt32(self.d.as_mut_ptr(), x);
        }
    }
    pub fn set_str(&mut self, s: &str, base: i32) -> bool {
        unsafe { mclBnFr_setStr(self.d.as_mut_ptr(), s.as_ptr(), s.len(), base as c_int) == 0 }
    }
    pub fn get_str(&self, io_mode: i32) -> String {
        let mut buf: [u8; 1024] = unsafe { MaybeUninit::uninit().assume_init() };
        let n: usize;
        unsafe {
            n = mclBnFr_getStr(
                buf.as_mut_ptr(),
                buf.len(),
                self.d.as_ptr(),
                io_mode as c_int,
            );
        }
        if n == 0 {
            panic!("mclBnFr_getStr");
        }
        buf[0..n].iter().map(|&s| s as char).collect::<String>()
    }
    pub fn get_str2(&self, io_mode: i32) -> String {
        let mut buf: Vec<u8> = Vec::with_capacity(1024);
        let n: usize;
        unsafe {
            n = mclBnFr_getStr(
                buf.as_mut_ptr(),
                buf.capacity(),
                self.d.as_ptr(),
                io_mode as c_int,
            );
        }
        if n == 0 {
            panic!("mclBnFr_getStr");
        }
        unsafe {
            buf.set_len(n);
            String::from_utf8_unchecked(buf)
        }
    }
    /*
        pub fn serialize(&self) -> Vec<u8> {
            let size = unsafe { mclBn_getFrByteSize() } as usize;
            make_serialize(size, &self.d, mclBnFr_serialize)
        }
    */
    pub fn deserialize(&mut self, buf: &[u8]) -> bool {
        unsafe { mclBnFr_deserialize(self.d.as_mut_ptr(), buf.as_ptr(), buf.len()) > 0 }
    }
}
