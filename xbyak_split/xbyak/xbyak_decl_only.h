#pragma once
#ifndef XBYAK_XBYAK_H_
#define XBYAK_XBYAK_H_
/*!
	@file xbyak.h
	@brief Xbyak ; JIT assembler for x86(IA32)/x64 by C++
	@author herumi
	@url https://github.com/herumi/xbyak
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#if (not +0) && !defined(XBYAK_NO_OP_NAMES) // trick to detect whether 'not' is operator or not
	#define XBYAK_NO_OP_NAMES
#endif

#include <stdio.h> // for debug print
#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#ifndef NDEBUG
#include <iostream>
#endif

// #define XBYAK_DISABLE_AVX512

#if !defined(XBYAK_USE_MMAP_ALLOCATOR) && !defined(XBYAK_DONT_USE_MMAP_ALLOCATOR)
	#define XBYAK_USE_MMAP_ALLOCATOR
#endif
#if !defined(__GNUC__) || defined(__MINGW32__)
	#undef XBYAK_USE_MMAP_ALLOCATOR
#endif

#ifdef __GNUC__
	#define XBYAK_GNUC_PREREQ(major, minor) ((__GNUC__) * 100 + (__GNUC_MINOR__) >= (major) * 100 + (minor))
#else
	#define XBYAK_GNUC_PREREQ(major, minor) 0
#endif

// This covers -std=(gnu|c)++(0x|11|1y), -stdlib=libc++, and modern Microsoft.
#if ((defined(_MSC_VER) && (_MSC_VER >= 1600)) || defined(_LIBCPP_VERSION) ||\
	 			 ((__cplusplus >= 201103) || defined(__GXX_EXPERIMENTAL_CXX0X__)))
	#include <unordered_set>
	#define XBYAK_STD_UNORDERED_SET std::unordered_set
	#include <unordered_map>
	#define XBYAK_STD_UNORDERED_MAP std::unordered_map
	#define XBYAK_STD_UNORDERED_MULTIMAP std::unordered_multimap

/*
	Clang/llvm-gcc and ICC-EDG in 'GCC-mode' always claim to be GCC 4.2, using
	libstdcxx 20070719 (from GCC 4.2.1, the last GPL 2 version).
*/
#elif XBYAK_GNUC_PREREQ(4, 5) || (XBYAK_GNUC_PREREQ(4, 2) && __GLIBCXX__ >= 20070719) || defined(__INTEL_COMPILER) || defined(__llvm__)
	#include <tr1/unordered_set>
	#define XBYAK_STD_UNORDERED_SET std::tr1::unordered_set
	#include <tr1/unordered_map>
	#define XBYAK_STD_UNORDERED_MAP std::tr1::unordered_map
	#define XBYAK_STD_UNORDERED_MULTIMAP std::tr1::unordered_multimap

#elif defined(_MSC_VER) && (_MSC_VER >= 1500) && (_MSC_VER < 1600)
	#include <unordered_set>
	#define XBYAK_STD_UNORDERED_SET std::tr1::unordered_set
	#include <unordered_map>
	#define XBYAK_STD_UNORDERED_MAP std::tr1::unordered_map
	#define XBYAK_STD_UNORDERED_MULTIMAP std::tr1::unordered_multimap

#else
	#include <set>
	#define XBYAK_STD_UNORDERED_SET std::set
	#include <map>
	#define XBYAK_STD_UNORDERED_MAP std::map
	#define XBYAK_STD_UNORDERED_MULTIMAP std::multimap
#endif
#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <malloc.h>
	#define XBYAK_TLS __declspec(thread)
#elif defined(__GNUC__)
	#include <unistd.h>
	#include <sys/mman.h>
	#include <stdlib.h>
	#define XBYAK_TLS __thread
#endif
#if defined(__APPLE__) && !defined(XBYAK_DONT_USE_MAP_JIT)
	#define XBYAK_USE_MAP_JIT
	#include <sys/sysctl.h>
	#ifndef MAP_JIT
		#define MAP_JIT 0x800
	#endif
#endif
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
	#include <stdint.h>
#endif

#if defined(_WIN64) || defined(__MINGW64__) || (defined(__CYGWIN__) && defined(__x86_64__))
	#define XBYAK64_WIN
#elif defined(__x86_64__)
	#define XBYAK64_GCC
#endif
#if !defined(XBYAK64) && !defined(XBYAK32)
	#if defined(XBYAK64_GCC) || defined(XBYAK64_WIN)
		#define XBYAK64
	#else
		#define XBYAK32
	#endif
#endif

#if (__cplusplus >= 201103) || (_MSC_VER >= 1800)
	#undef XBYAK_TLS
	#define XBYAK_TLS thread_local
	#define XBYAK_VARIADIC_TEMPLATE
#endif

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4514) /* remove inline function */
	#pragma warning(disable : 4786) /* identifier is too long */
	#pragma warning(disable : 4503) /* name is too long */
	#pragma warning(disable : 4127) /* constant expresison */
#endif

namespace Xbyak {

enum {
	DEFAULT_MAX_CODE_SIZE = 4096,
	VERSION = 0x5941 /* 0xABCD = A.BC(D) */
};

#ifndef MIE_INTEGER_TYPE_DEFINED
#define MIE_INTEGER_TYPE_DEFINED
#ifdef _MSC_VER
	typedef unsigned __int64 uint64;
	typedef __int64 sint64;
#else
	typedef uint64_t uint64;
	typedef int64_t sint64;
#endif
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#endif

#ifndef MIE_ALIGN
	#ifdef _MSC_VER
		#define MIE_ALIGN(x) __declspec(align(x))
	#else
		#define MIE_ALIGN(x) __attribute__((aligned(x)))
	#endif
#endif
#ifndef MIE_PACK // for shufps
	#define MIE_PACK(x, y, z, w) ((x) * 64 + (y) * 16 + (z) * 4 + (w))
#endif

enum {
	ERR_NONE = 0,
	ERR_BAD_ADDRESSING,
	ERR_CODE_IS_TOO_BIG,
	ERR_BAD_SCALE,
	ERR_ESP_CANT_BE_INDEX,
	ERR_BAD_COMBINATION,
	ERR_BAD_SIZE_OF_REGISTER,
	ERR_IMM_IS_TOO_BIG,
	ERR_BAD_ALIGN,
	ERR_LABEL_IS_REDEFINED,
	ERR_LABEL_IS_TOO_FAR,
	ERR_LABEL_IS_NOT_FOUND,
	ERR_CODE_ISNOT_COPYABLE,
	ERR_BAD_PARAMETER,
	ERR_CANT_PROTECT,
	ERR_CANT_USE_64BIT_DISP,
	ERR_OFFSET_IS_TOO_BIG,
	ERR_MEM_SIZE_IS_NOT_SPECIFIED,
	ERR_BAD_MEM_SIZE,
	ERR_BAD_ST_COMBINATION,
	ERR_OVER_LOCAL_LABEL, // not used
	ERR_UNDER_LOCAL_LABEL,
	ERR_CANT_ALLOC,
	ERR_ONLY_T_NEAR_IS_SUPPORTED_IN_AUTO_GROW,
	ERR_BAD_PROTECT_MODE,
	ERR_BAD_PNUM,
	ERR_BAD_TNUM,
	ERR_BAD_VSIB_ADDRESSING,
	ERR_CANT_CONVERT,
	ERR_LABEL_ISNOT_SET_BY_L,
	ERR_LABEL_IS_ALREADY_SET_BY_L,
	ERR_BAD_LABEL_STR,
	ERR_MUNMAP,
	ERR_OPMASK_IS_ALREADY_SET,
	ERR_ROUNDING_IS_ALREADY_SET,
	ERR_K0_IS_INVALID,
	ERR_EVEX_IS_INVALID,
	ERR_SAE_IS_INVALID,
	ERR_ER_IS_INVALID,
	ERR_INVALID_BROADCAST,
	ERR_INVALID_OPMASK_WITH_MEMORY,
	ERR_INVALID_ZERO,
	ERR_INVALID_RIP_IN_AUTO_GROW,
	ERR_INVALID_MIB_ADDRESS,
	ERR_X2APIC_IS_NOT_SUPPORTED,
	ERR_NOT_SUPPORTED,
	ERR_INTERNAL // Put it at last.
};

inline const char *ConvertErrorToString(int err)
{
	static const char *errTbl[] = {
		"none",
		"bad addressing",
		"code is too big",
		"bad scale",
		"esp can't be index",
		"bad combination",
		"bad size of register",
		"imm is too big",
		"bad align",
		"label is redefined",
		"label is too far",
		"label is not found",
		"code is not copyable",
		"bad parameter",
		"can't protect",
		"can't use 64bit disp(use (void*))",
		"offset is too big",
		"MEM size is not specified",
		"bad mem size",
		"bad st combination",
		"over local label",
		"under local label",
		"can't alloc",
		"T_SHORT is not supported in AutoGrow",
		"bad protect mode",
		"bad pNum",
		"bad tNum",
		"bad vsib addressing",
		"can't convert",
		"label is not set by L()",
		"label is already set by L()",
		"bad label string",
		"err munmap",
		"opmask is already set",
		"rounding is already set",
		"k0 is invalid",
		"evex is invalid",
		"sae(suppress all exceptions) is invalid",
		"er(embedded rounding) is invalid",
		"invalid broadcast",
		"invalid opmask with memory",
		"invalid zero",
		"invalid rip in AutoGrow",
		"invalid mib address",
		"x2APIC is not supported",
		"not supported",
		"internal error"
	};
	assert(ERR_INTERNAL + 1 == sizeof(errTbl) / sizeof(*errTbl));
	return err <= ERR_INTERNAL ? errTbl[err] : "unknown err";
}

#ifdef XBYAK_NO_EXCEPTION
namespace local {

static XBYAK_TLS int l_err = 0;
inline void SetError(int err) { if (err) l_err = err; } // keep the first err code

} // local

inline void ClearError() { local::l_err = 0; }
inline int GetError() { return local::l_err; }

#define XBYAK_THROW(err) { local::SetError(err); return; }
#define XBYAK_THROW_RET(err, r) { local::SetError(err); return r; }

#else
class Error : public std::exception {
	int err_;
public:
	explicit Error(int err) : err_(err)
	{
		if (err_ < 0 || err_ > ERR_INTERNAL) {
			err_ = ERR_INTERNAL;
		}
	}
	operator int() const { return err_; }
	const char *what() const throw()
	{
		return ConvertErrorToString(err_);
	}
};

// dummy functions
inline void ClearError() { }
inline int GetError() { return 0; }

inline const char *ConvertErrorToString(const Error& err)
{
	return err.what();
}

#define XBYAK_THROW(err) { throw Error(err); }
#define XBYAK_THROW_RET(err, r) { throw Error(err); }

#endif

inline void *AlignedMalloc(size_t size, size_t alignment)
{
#ifdef __MINGW32__
	return __mingw_aligned_malloc(size, alignment);
#elif defined(_WIN32)
	return _aligned_malloc(size, alignment);
#else
	void *p;
	int ret = posix_memalign(&p, alignment, size);
	return (ret == 0) ? p : 0;
#endif
}

inline void AlignedFree(void *p)
{
#ifdef __MINGW32__
	__mingw_aligned_free(p);
#elif defined(_MSC_VER)
	_aligned_free(p);
#else
	free(p);
#endif
}

template<class To, class From>
inline const To CastTo(From p) throw()
{
	return (const To)(size_t)(p);
}
namespace inner {

static const size_t ALIGN_PAGE_SIZE = 4096;

inline bool IsInDisp8(uint32 x) { return 0xFFFFFF80 <= x || x <= 0x7F; }
inline bool IsInInt32(uint64 x) { return ~uint64(0x7fffffffu) <= x || x <= 0x7FFFFFFFU; }

inline uint32 VerifyInInt32(uint64 x)
{
#ifdef XBYAK64
	if (!IsInInt32(x)) XBYAK_THROW_RET(ERR_OFFSET_IS_TOO_BIG, 0)
#endif
	return static_cast<uint32>(x);
}

enum LabelMode {
	LasIs, // as is
	Labs, // absolute
	LaddTop // (addr + top) for mov(reg, label) with AutoGrow
};

} // inner

/*
	custom allocator
*/
struct Allocator {
	virtual uint8 *alloc(size_t size) { return reinterpret_cast<uint8*>(AlignedMalloc(size, inner::ALIGN_PAGE_SIZE)); }
	virtual void free(uint8 *p) { AlignedFree(p); }
	virtual ~Allocator() {}
	/* override to return false if you call protect() manually */
	virtual bool useProtect() const { return true; }
};

#ifdef XBYAK_USE_MMAP_ALLOCATOR
#ifdef XBYAK_USE_MAP_JIT
namespace util {

inline int getMacOsVersionPure()
{
	char buf[64];
	size_t size = sizeof(buf);
	int err = sysctlbyname("kern.osrelease", buf, &size, NULL, 0);
	if (err != 0) return 0;
	char *endp;
	int major = strtol(buf, &endp, 10);
	if (*endp != '.') return 0;
	return major;
}

inline int getMacOsVersion()
{
	static const int version = getMacOsVersionPure();
	return version;
}

} // util
#endif
class MmapAllocator : Allocator {
	typedef XBYAK_STD_UNORDERED_MAP<uintptr_t, size_t> SizeList;
	SizeList sizeList_;
public:
	uint8 *alloc(size_t size)
	{
		const size_t alignedSizeM1 = inner::ALIGN_PAGE_SIZE - 1;
		size = (size + alignedSizeM1) & ~alignedSizeM1;
#if defined(XBYAK_USE_MAP_JIT)
		int mode = MAP_PRIVATE | MAP_ANONYMOUS;
		const int mojaveVersion = 18;
		if (util::getMacOsVersion() >= mojaveVersion) mode |= MAP_JIT;
#elif defined(MAP_ANONYMOUS)
		const int mode = MAP_PRIVATE | MAP_ANONYMOUS;
#elif defined(MAP_ANON)
		const int mode = MAP_PRIVATE | MAP_ANON;
#else
		#error "not supported"
#endif
		void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, mode, -1, 0);
		if (p == MAP_FAILED) XBYAK_THROW_RET(ERR_CANT_ALLOC, 0)
		assert(p);
		sizeList_[(uintptr_t)p] = size;
		return (uint8*)p;
	}
	void free(uint8 *p)
	{
		if (p == 0) return;
		SizeList::iterator i = sizeList_.find((uintptr_t)p);
		if (i == sizeList_.end()) XBYAK_THROW(ERR_BAD_PARAMETER)
		if (munmap((void*)i->first, i->second) < 0) XBYAK_THROW(ERR_MUNMAP)
		sizeList_.erase(i);
	}
};
#endif

class Address;
class Reg;

class Operand {
	static const uint8 EXT8BIT = 0x20;
	unsigned int idx_:6; // 0..31 + EXT8BIT = 1 if spl/bpl/sil/dil
	unsigned int kind_:10;
	unsigned int bit_:14;
protected:
	unsigned int zero_:1;
	unsigned int mask_:3;
	unsigned int rounding_:3;
	void setIdx(int idx) { idx_ = idx; }
public:
	enum Kind {
		NONE = 0,
		MEM = 1 << 0,
		REG = 1 << 1,
		MMX = 1 << 2,
		FPU = 1 << 3,
		XMM = 1 << 4,
		YMM = 1 << 5,
		ZMM = 1 << 6,
		OPMASK = 1 << 7,
		BNDREG = 1 << 8,
		TMM = 1 << 9
	};
	enum Code {
#ifdef XBYAK64
		RAX = 0, RCX, RDX, RBX, RSP, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15,
		R8D = 8, R9D, R10D, R11D, R12D, R13D, R14D, R15D,
		R8W = 8, R9W, R10W, R11W, R12W, R13W, R14W, R15W,
		R8B = 8, R9B, R10B, R11B, R12B, R13B, R14B, R15B,
		SPL = 4, BPL, SIL, DIL,
#endif
		EAX = 0, ECX, EDX, EBX, ESP, EBP, ESI, EDI,
		AX = 0, CX, DX, BX, SP, BP, SI, DI,
		AL = 0, CL, DL, BL, AH, CH, DH, BH
	};
	Operand() : idx_(0), kind_(0), bit_(0), zero_(0), mask_(0), rounding_(0) { }
	Operand(int idx, Kind kind, int bit, bool ext8bit = 0)
		: idx_(static_cast<uint8>(idx | (ext8bit ? EXT8BIT : 0)))
		, kind_(kind)
		, bit_(bit)
		, zero_(0), mask_(0), rounding_(0)
	{
		assert((bit_ & (bit_ - 1)) == 0); // bit must be power of two
	}
	Kind getKind() const { return static_cast<Kind>(kind_); }
	int getIdx() const { return idx_ & (EXT8BIT - 1); }
	bool isNone() const { return kind_ == 0; }
	bool isMMX() const { return is(MMX); }
	bool isXMM() const { return is(XMM); }
	bool isYMM() const { return is(YMM); }
	bool isZMM() const { return is(ZMM); }
	bool isTMM() const { return is(TMM); }
	bool isXMEM() const { return is(XMM | MEM); }
	bool isYMEM() const { return is(YMM | MEM); }
	bool isZMEM() const { return is(ZMM | MEM); }
	bool isOPMASK() const { return is(OPMASK); }
	bool isBNDREG() const { return is(BNDREG); }
	bool isREG(int bit = 0) const { return is(REG, bit); }
	bool isMEM(int bit = 0) const { return is(MEM, bit); }
	bool isFPU() const { return is(FPU); }
	bool isExt8bit() const { return (idx_ & EXT8BIT) != 0; }
	bool isExtIdx() const { return (getIdx() & 8) != 0; }
	bool isExtIdx2() const { return (getIdx() & 16) != 0; }
	bool hasEvex() const { return isZMM() || isExtIdx2() || getOpmaskIdx() || getRounding(); }
	bool hasRex() const { return isExt8bit() || isREG(64) || isExtIdx(); }
	bool hasZero() const { return zero_; }
	int getOpmaskIdx() const { return mask_; }
	int getRounding() const { return rounding_; }
	void setKind(Kind kind)
	{
		if ((kind & (XMM|YMM|ZMM|TMM)) == 0) return;
		kind_ = kind;
		bit_ = kind == XMM ? 128 : kind == YMM ? 256 : kind == ZMM ? 512 : 8192;
	}
	// err if MMX/FPU/OPMASK/BNDREG
	void setBit(int bit);
	void setOpmaskIdx(int idx, bool /*ignore_idx0*/ = true)
	{
		if (mask_) XBYAK_THROW(ERR_OPMASK_IS_ALREADY_SET)
		mask_ = idx;
	}
	void setRounding(int idx)
	{
		if (rounding_) XBYAK_THROW(ERR_ROUNDING_IS_ALREADY_SET)
		rounding_ = idx;
	}
	void setZero() { zero_ = true; }
	// ah, ch, dh, bh?
	bool isHigh8bit() const
	{
		if (!isBit(8)) return false;
		if (isExt8bit()) return false;
		const int idx = getIdx();
		return AH <= idx && idx <= BH;
	}
	// any bit is accetable if bit == 0
	bool is(int kind, uint32 bit = 0) const
	{
		return (kind == 0 || (kind_ & kind)) && (bit == 0 || (bit_ & bit)); // cf. you can set (8|16)
	}
	bool isBit(uint32 bit) const { return (bit_ & bit) != 0; }
	uint32 getBit() const { return bit_; }
	const char *toString() const
	{
		const int idx = getIdx();
		if (kind_ == REG) {
			if (isExt8bit()) {
				static const char *tbl[4] = { "spl", "bpl", "sil", "dil" };
				return tbl[idx - 4];
			}
			static const char *tbl[4][16] = {
				{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "r8b", "r9b", "r10b",  "r11b", "r12b", "r13b", "r14b", "r15b" },
				{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w",  "r11w", "r12w", "r13w", "r14w", "r15w" },
				{ "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d",  "r11d", "r12d", "r13d", "r14d", "r15d" },
				{ "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15" },
			};
			return tbl[bit_ == 8 ? 0 : bit_ == 16 ? 1 : bit_ == 32 ? 2 : 3][idx];
		} else if (isOPMASK()) {
			static const char *tbl[8] = { "k0", "k1", "k2", "k3", "k4", "k5", "k6", "k7" };
			return tbl[idx];
		} else if (isTMM()) {
			static const char *tbl[8] = {
				"tmm0", "tmm1", "tmm2", "tmm3", "tmm4", "tmm5", "tmm6", "tmm7"
			};
			return tbl[idx];
		} else if (isZMM()) {
			static const char *tbl[32] = {
				"zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15",
				"zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29", "zmm30", "zmm31"
			};
			return tbl[idx];
		} else if (isYMM()) {
			static const char *tbl[32] = {
				"ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
				"ymm16", "ymm17", "ymm18", "ymm19", "ymm20", "ymm21", "ymm22", "ymm23", "ymm24", "ymm25", "ymm26", "ymm27", "ymm28", "ymm29", "ymm30", "ymm31"
			};
			return tbl[idx];
		} else if (isXMM()) {
			static const char *tbl[32] = {
				"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
				"xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31"
			};
			return tbl[idx];
		} else if (isMMX()) {
			static const char *tbl[8] = { "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7" };
			return tbl[idx];
		} else if (isFPU()) {
			static const char *tbl[8] = { "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7" };
			return tbl[idx];
		} else if (isBNDREG()) {
			static const char *tbl[4] = { "bnd0", "bnd1", "bnd2", "bnd3" };
			return tbl[idx];
		}
		XBYAK_THROW_RET(ERR_INTERNAL, 0);
	}
	bool isEqualIfNotInherited(const Operand& rhs) const { return idx_ == rhs.idx_ && kind_ == rhs.kind_ && bit_ == rhs.bit_ && zero_ == rhs.zero_ && mask_ == rhs.mask_ && rounding_ == rhs.rounding_; }
	bool operator==(const Operand& rhs) const;
	bool operator!=(const Operand& rhs) const { return !operator==(rhs); }
	const Address& getAddress() const;
	const Reg& getReg() const;
};

inline void Operand::setBit(int bit)
{
	if (bit != 8 && bit != 16 && bit != 32 && bit != 64 && bit != 128 && bit != 256 && bit != 512 && bit != 8192) goto ERR;
	if (isBit(bit)) return;
	if (is(MEM | OPMASK)) {
		bit_ = bit;
		return;
	}
	if (is(REG | XMM | YMM | ZMM | TMM)) {
		int idx = getIdx();
		// err if converting ah, bh, ch, dh
		if (isREG(8) && (4 <= idx && idx < 8) && !isExt8bit()) goto ERR;
		Kind kind = REG;
		switch (bit) {
		case 8:
			if (idx >= 16) goto ERR;
#ifdef XBYAK32
			if (idx >= 4) goto ERR;
#else
			if (4 <= idx && idx < 8) idx |= EXT8BIT;
#endif
			break;
		case 16:
		case 32:
		case 64:
			if (idx >= 16) goto ERR;
			break;
		case 128: kind = XMM; break;
		case 256: kind = YMM; break;
		case 512: kind = ZMM; break;
		case 8192: kind = TMM; break;
		}
		idx_ = idx;
		kind_ = kind;
		bit_ = bit;
		if (bit >= 128) return; // keep mask_ and rounding_
		mask_ = 0;
		rounding_ = 0;
		return;
	}
ERR:
	XBYAK_THROW(ERR_CANT_CONVERT)
}

class Label;

struct Reg8;
struct Reg16;
struct Reg32;
#ifdef XBYAK64
struct Reg64;
#endif
class Reg : public Operand {
public:
	Reg() { }
	Reg(int idx, Kind kind, int bit = 0, bool ext8bit = false) : Operand(idx, kind, bit, ext8bit) { }
	// convert to Reg8/Reg16/Reg32/Reg64/XMM/YMM/ZMM
	Reg changeBit(int bit) const { Reg r(*this); r.setBit(bit); return r; }
	uint8 getRexW() const { return isREG(64) ? 8 : 0; }
	uint8 getRexR() const { return isExtIdx() ? 4 : 0; }
	uint8 getRexX() const { return isExtIdx() ? 2 : 0; }
	uint8 getRexB() const { return isExtIdx() ? 1 : 0; }
	uint8 getRex(const Reg& base = Reg()) const
	{
		uint8 rex = getRexW() | getRexR() | base.getRexW() | base.getRexB();
		if (rex || isExt8bit() || base.isExt8bit()) rex |= 0x40;
		return rex;
	}
	Reg8 cvt8() const;
	Reg16 cvt16() const;
	Reg32 cvt32() const;
#ifdef XBYAK64
	Reg64 cvt64() const;
#endif
};

inline const Reg& Operand::getReg() const
{
	assert(!isMEM());
	return static_cast<const Reg&>(*this);
}

struct Reg8 : public Reg {
	explicit Reg8(int idx = 0, bool ext8bit = false) : Reg(idx, Operand::REG, 8, ext8bit) { }
};

struct Reg16 : public Reg {
	explicit Reg16(int idx = 0) : Reg(idx, Operand::REG, 16) { }
};

struct Mmx : public Reg {
	explicit Mmx(int idx = 0, Kind kind = Operand::MMX, int bit = 64) : Reg(idx, kind, bit) { }
};

struct EvexModifierRounding {
	enum {
		T_RN_SAE = 1,
		T_RD_SAE = 2,
		T_RU_SAE = 3,
		T_RZ_SAE = 4,
		T_SAE = 5
	};
	explicit EvexModifierRounding(int rounding) : rounding(rounding) {}
	int rounding;
};
struct EvexModifierZero{EvexModifierZero() {}};

struct Xmm : public Mmx {
	explicit Xmm(int idx = 0, Kind kind = Operand::XMM, int bit = 128) : Mmx(idx, kind, bit) { }
	Xmm(Kind kind, int idx) : Mmx(idx, kind, kind == XMM ? 128 : kind == YMM ? 256 : 512) { }
	Xmm operator|(const EvexModifierRounding& emr) const { Xmm r(*this); r.setRounding(emr.rounding); return r; }
	Xmm copyAndSetIdx(int idx) const { Xmm ret(*this); ret.setIdx(idx); return ret; }
	Xmm copyAndSetKind(Operand::Kind kind) const { Xmm ret(*this); ret.setKind(kind); return ret; }
};

struct Ymm : public Xmm {
	explicit Ymm(int idx = 0, Kind kind = Operand::YMM, int bit = 256) : Xmm(idx, kind, bit) { }
	Ymm operator|(const EvexModifierRounding& emr) const { Ymm r(*this); r.setRounding(emr.rounding); return r; }
};

struct Zmm : public Ymm {
	explicit Zmm(int idx = 0) : Ymm(idx, Operand::ZMM, 512) { }
	Zmm operator|(const EvexModifierRounding& emr) const { Zmm r(*this); r.setRounding(emr.rounding); return r; }
};

#ifdef XBYAK64
struct Tmm : public Reg {
	explicit Tmm(int idx = 0, Kind kind = Operand::TMM, int bit = 8192) : Reg(idx, kind, bit) { }
};
#endif

struct Opmask : public Reg {
	explicit Opmask(int idx = 0) : Reg(idx, Operand::OPMASK, 64) {}
};

struct BoundsReg : public Reg {
	explicit BoundsReg(int idx = 0) : Reg(idx, Operand::BNDREG, 128) {}
};

template<class T>T operator|(const T& x, const Opmask& k) { T r(x); r.setOpmaskIdx(k.getIdx()); return r; }
template<class T>T operator|(const T& x, const EvexModifierZero&) { T r(x); r.setZero(); return r; }
template<class T>T operator|(const T& x, const EvexModifierRounding& emr) { T r(x); r.setRounding(emr.rounding); return r; }

struct Fpu : public Reg {
	explicit Fpu(int idx = 0) : Reg(idx, Operand::FPU, 32) { }
};

struct Reg32e : public Reg {
	explicit Reg32e(int idx, int bit) : Reg(idx, Operand::REG, bit) {}
};
struct Reg32 : public Reg32e {
	explicit Reg32(int idx = 0) : Reg32e(idx, 32) {}
};
#ifdef XBYAK64
struct Reg64 : public Reg32e {
	explicit Reg64(int idx = 0) : Reg32e(idx, 64) {}
};
struct RegRip {
	sint64 disp_;
	const Label* label_;
	bool isAddr_;
	explicit RegRip(sint64 disp = 0, const Label* label = 0, bool isAddr = false) : disp_(disp), label_(label), isAddr_(isAddr) {}
	friend const RegRip operator+(const RegRip& r, int disp) {
		return RegRip(r.disp_ + disp, r.label_, r.isAddr_);
	}
	friend const RegRip operator-(const RegRip& r, int disp) {
		return RegRip(r.disp_ - disp, r.label_, r.isAddr_);
	}
	friend const RegRip operator+(const RegRip& r, sint64 disp) {
		return RegRip(r.disp_ + disp, r.label_, r.isAddr_);
	}
	friend const RegRip operator-(const RegRip& r, sint64 disp) {
		return RegRip(r.disp_ - disp, r.label_, r.isAddr_);
	}
	friend const RegRip operator+(const RegRip& r, const Label& label) {
		if (r.label_ || r.isAddr_) XBYAK_THROW_RET(ERR_BAD_ADDRESSING, RegRip());
		return RegRip(r.disp_, &label);
	}
	friend const RegRip operator+(const RegRip& r, const void *addr) {
		if (r.label_ || r.isAddr_) XBYAK_THROW_RET(ERR_BAD_ADDRESSING, RegRip());
		return RegRip(r.disp_ + (sint64)addr, 0, true);
	}
};
#endif

inline Reg8 Reg::cvt8() const
{
	Reg r = changeBit(8); return Reg8(r.getIdx(), r.isExt8bit());
}

inline Reg16 Reg::cvt16() const
{
	return Reg16(changeBit(16).getIdx());
}

inline Reg32 Reg::cvt32() const
{
	return Reg32(changeBit(32).getIdx());
}

#ifdef XBYAK64
inline Reg64 Reg::cvt64() const
{
	return Reg64(changeBit(64).getIdx());
}
#endif

#ifndef XBYAK_DISABLE_SEGMENT
// not derived from Reg
class Segment {
	int idx_;
public:
	enum {
		es, cs, ss, ds, fs, gs
	};
	explicit Segment(int idx) : idx_(idx) { assert(0 <= idx_ && idx_ < 6); }
	int getIdx() const { return idx_; }
	const char *toString() const
	{
		static const char tbl[][3] = {
			"es", "cs", "ss", "ds", "fs", "gs"
		};
		return tbl[idx_];
	}
};
#endif

class RegExp {
public:
#ifdef XBYAK64
	enum { i32e = 32 | 64 };
#else
	enum { i32e = 32 };
#endif
	RegExp(size_t disp = 0) : scale_(0), disp_(disp) { }
	RegExp(const Reg& r, int scale = 1)
		: scale_(scale)
		, disp_(0)
	{
		if (!r.isREG(i32e) && !r.is(Reg::XMM|Reg::YMM|Reg::ZMM|Reg::TMM)) XBYAK_THROW(ERR_BAD_SIZE_OF_REGISTER)
		if (scale == 0) return;
		if (scale != 1 && scale != 2 && scale != 4 && scale != 8) XBYAK_THROW(ERR_BAD_SCALE)
		if (r.getBit() >= 128 || scale != 1) { // xmm/ymm is always index
			index_ = r;
		} else {
			base_ = r;
		}
	}
	bool isVsib(int bit = 128 | 256 | 512) const { return index_.isBit(bit); }
	RegExp optimize() const
	{
		RegExp exp = *this;
		// [reg * 2] => [reg + reg]
		if (index_.isBit(i32e) && !base_.getBit() && scale_ == 2) {
			exp.base_ = index_;
			exp.scale_ = 1;
		}
		return exp;
	}
	bool operator==(const RegExp& rhs) const
	{
		return base_ == rhs.base_ && index_ == rhs.index_ && disp_ == rhs.disp_ && scale_ == rhs.scale_;
	}
	const Reg& getBase() const { return base_; }
	const Reg& getIndex() const { return index_; }
	int getScale() const { return scale_; }
	size_t getDisp() const { return disp_; }
	void verify() const
	{
		if (base_.getBit() >= 128) XBYAK_THROW(ERR_BAD_SIZE_OF_REGISTER)
		if (index_.getBit() && index_.getBit() <= 64) {
			if (index_.getIdx() == Operand::ESP) XBYAK_THROW(ERR_ESP_CANT_BE_INDEX)
			if (base_.getBit() && base_.getBit() != index_.getBit()) XBYAK_THROW(ERR_BAD_SIZE_OF_REGISTER)
		}
	}
	friend RegExp operator+(const RegExp& a, const RegExp& b);
	friend RegExp operator-(const RegExp& e, size_t disp);
	uint8 getRex() const
	{
		uint8 rex = index_.getRexX() | base_.getRexB();
		return rex ? uint8(rex | 0x40) : 0;
	}
private:
	/*
		[base_ + index_ * scale_ + disp_]
		base : Reg32e, index : Reg32e(w/o esp), Xmm, Ymm
	*/
	Reg base_;
	Reg index_;
	int scale_;
	size_t disp_;
};

inline RegExp operator+(const RegExp& a, const RegExp& b)
{
	if (a.index_.getBit() && b.index_.getBit()) XBYAK_THROW_RET(ERR_BAD_ADDRESSING, RegExp())
	RegExp ret = a;
	if (!ret.index_.getBit()) { ret.index_ = b.index_; ret.scale_ = b.scale_; }
	if (b.base_.getBit()) {
		if (ret.base_.getBit()) {
			if (ret.index_.getBit()) XBYAK_THROW_RET(ERR_BAD_ADDRESSING, RegExp())
			// base + base => base + index * 1
			ret.index_ = b.base_;
			// [reg + esp] => [esp + reg]
			if (ret.index_.getIdx() == Operand::ESP) std::swap(ret.base_, ret.index_);
			ret.scale_ = 1;
		} else {
			ret.base_ = b.base_;
		}
	}
	ret.disp_ += b.disp_;
	return ret;
}
inline RegExp operator*(const Reg& r, int scale)
{
	return RegExp(r, scale);
}
inline RegExp operator-(const RegExp& e, size_t disp)
{
	RegExp ret = e;
	ret.disp_ -= disp;
	return ret;
}

// 2nd parameter for constructor of CodeArray(maxSize, userPtr, alloc)
void *const AutoGrow = (void*)1; //-V566
void *const DontSetProtectRWE = (void*)2; //-V566

class CodeArray {
	enum Type {
		USER_BUF = 1, // use userPtr(non alignment, non protect)
		ALLOC_BUF, // use new(alignment, protect)
		AUTO_GROW // automatically move and grow memory if necessary
	};
	CodeArray(const CodeArray& rhs);
	void operator=(const CodeArray&);
	bool isAllocType() const { return type_ == ALLOC_BUF || type_ == AUTO_GROW; }
	struct AddrInfo {
		size_t codeOffset; // position to write
		size_t jmpAddr; // value to write
		int jmpSize; // size of jmpAddr
		inner::LabelMode mode;
		AddrInfo(size_t _codeOffset, size_t _jmpAddr, int _jmpSize, inner::LabelMode _mode)
			: codeOffset(_codeOffset), jmpAddr(_jmpAddr), jmpSize(_jmpSize), mode(_mode) {}
		uint64 getVal(const uint8 *top) const
		{
			uint64 disp = (mode == inner::LaddTop) ? jmpAddr + size_t(top) : (mode == inner::LasIs) ? jmpAddr : jmpAddr - size_t(top);
			if (jmpSize == 4) disp = inner::VerifyInInt32(disp);
			return disp;
		}
	};
	typedef std::list<AddrInfo> AddrInfoList;
	AddrInfoList addrInfoList_;
	const Type type_;
#ifdef XBYAK_USE_MMAP_ALLOCATOR
	MmapAllocator defaultAllocator_;
#else
	Allocator defaultAllocator_;
#endif
	Allocator *alloc_;
protected:
	size_t maxSize_;
	uint8 *top_;
	size_t size_;
	bool isCalledCalcJmpAddress_;

	bool useProtect() const { return alloc_->useProtect(); }
	/*
		allocate new memory and copy old data to the new area
	*/
	void growMemory()
	{
		const size_t newSize = (std::max<size_t>)(DEFAULT_MAX_CODE_SIZE, maxSize_ * 2);
		uint8 *newTop = alloc_->alloc(newSize);
		if (newTop == 0) XBYAK_THROW(ERR_CANT_ALLOC)
		for (size_t i = 0; i < size_; i++) newTop[i] = top_[i];
		alloc_->free(top_);
		top_ = newTop;
		maxSize_ = newSize;
	}
	/*
		calc jmp address for AutoGrow mode
	*/
	void calcJmpAddress()
	{
		if (isCalledCalcJmpAddress_) return;
		for (AddrInfoList::const_iterator i = addrInfoList_.begin(), ie = addrInfoList_.end(); i != ie; ++i) {
			uint64 disp = i->getVal(top_);
			rewrite(i->codeOffset, disp, i->jmpSize);
		}
		isCalledCalcJmpAddress_ = true;
	}
public:
	enum ProtectMode {
		PROTECT_RW = 0, // read/write
		PROTECT_RWE = 1, // read/write/exec
		PROTECT_RE = 2 // read/exec
	};
	explicit CodeArray(size_t maxSize, void *userPtr = 0, Allocator *allocator = 0)
		: type_(userPtr == AutoGrow ? AUTO_GROW : (userPtr == 0 || userPtr == DontSetProtectRWE) ? ALLOC_BUF : USER_BUF)
		, alloc_(allocator ? allocator : (Allocator*)&defaultAllocator_)
		, maxSize_(maxSize)
		, top_(type_ == USER_BUF ? reinterpret_cast<uint8*>(userPtr) : alloc_->alloc((std::max<size_t>)(maxSize, 1)))
		, size_(0)
		, isCalledCalcJmpAddress_(false)
	{
		if (maxSize_ > 0 && top_ == 0) XBYAK_THROW(ERR_CANT_ALLOC)
		if ((type_ == ALLOC_BUF && userPtr != DontSetProtectRWE && useProtect()) && !setProtectMode(PROTECT_RWE, false)) {
			alloc_->free(top_);
			XBYAK_THROW(ERR_CANT_PROTECT)
		}
	}
	virtual ~CodeArray()
	{
		if (isAllocType()) {
			if (useProtect()) setProtectModeRW(false);
			alloc_->free(top_);
		}
	}
	bool setProtectMode(ProtectMode mode, bool throwException = true)
	{
		bool isOK = protect(top_, maxSize_, mode);
		if (isOK) return true;
		if (throwException) XBYAK_THROW_RET(ERR_CANT_PROTECT, false)
		return false;
	}
	bool setProtectModeRE(bool throwException = true) { return setProtectMode(PROTECT_RE, throwException); }
	bool setProtectModeRW(bool throwException = true) { return setProtectMode(PROTECT_RW, throwException); }
	void resetSize()
	{
		size_ = 0;
		addrInfoList_.clear();
		isCalledCalcJmpAddress_ = false;
	}
	void db(int code)
	{
		if (size_ >= maxSize_) {
			if (type_ == AUTO_GROW) {
				growMemory();
			} else {
				XBYAK_THROW(ERR_CODE_IS_TOO_BIG)
			}
		}
		top_[size_++] = static_cast<uint8>(code);
	}
	void db(const uint8 *code, size_t codeSize)
	{
		for (size_t i = 0; i < codeSize; i++) db(code[i]);
	}
	void db(uint64 code, size_t codeSize)
	{
		if (codeSize > 8) XBYAK_THROW(ERR_BAD_PARAMETER)
		for (size_t i = 0; i < codeSize; i++) db(static_cast<uint8>(code >> (i * 8)));
	}
	void dw(uint32 code) { db(code, 2); }
	void dd(uint32 code) { db(code, 4); }
	void dq(uint64 code) { db(code, 8); }
	const uint8 *getCode() const { return top_; }
	template<class F>
	const F getCode() const { return reinterpret_cast<F>(top_); }
	const uint8 *getCurr() const { return &top_[size_]; }
	template<class F>
	const F getCurr() const { return reinterpret_cast<F>(&top_[size_]); }
	size_t getSize() const { return size_; }
	void setSize(size_t size)
	{
		if (size > maxSize_) XBYAK_THROW(ERR_OFFSET_IS_TOO_BIG)
		size_ = size;
	}
	void dump() const;
	/*
		@param offset [in] offset from top
		@param disp [in] offset from the next of jmp
		@param size [in] write size(1, 2, 4, 8)
	*/
	void rewrite(size_t offset, uint64 disp, size_t size);
	void save(size_t offset, size_t val, int size, inner::LabelMode mode);
	bool isAutoGrow() const { return type_ == AUTO_GROW; }
	bool isCalledCalcJmpAddress() const { return isCalledCalcJmpAddress_; }
	/**
		change exec permission of memory
		@param addr [in] buffer address
		@param size [in] buffer size
		@param protectMode [in] mode(RW/RWE/RE)
		@return true(success), false(failure)
	*/
	static bool protect(const void *addr, size_t size, int protectMode);
	/**
		get aligned memory pointer
		@param addr [in] address
		@param alignedSize [in] power of two
		@return aligned addr by alingedSize
	*/
	static uint8 *getAlignedAddress(uint8 *addr, size_t alignedSize = 16);
};

class Address : public Operand {
public:
	enum Mode {
		M_ModRM,
		M_64bitDisp,
		M_rip,
		M_ripAddr
	};
	Address(uint32 sizeBit, bool broadcast, const RegExp& e)
		: Operand(0, MEM, sizeBit), e_(e), label_(0), mode_(M_ModRM), broadcast_(broadcast)
	{
		e_.verify();
	}
#ifdef XBYAK64
	explicit Address(size_t disp)
		: Operand(0, MEM, 64), e_(disp), label_(0), mode_(M_64bitDisp), broadcast_(false){ }
	Address(uint32 sizeBit, bool broadcast, const RegRip& addr)
		: Operand(0, MEM, sizeBit), e_(addr.disp_), label_(addr.label_), mode_(addr.isAddr_ ? M_ripAddr : M_rip), broadcast_(broadcast) { }
#endif
	RegExp getRegExp(bool optimize = true) const
	{
		return optimize ? e_.optimize() : e_;
	}
	Mode getMode() const { return mode_; }
	bool is32bit() const { return e_.getBase().getBit() == 32 || e_.getIndex().getBit() == 32; }
	bool isOnlyDisp() const { return !e_.getBase().getBit() && !e_.getIndex().getBit(); } // for mov eax
	size_t getDisp() const { return e_.getDisp(); }
	uint8 getRex() const
	{
		if (mode_ != M_ModRM) return 0;
		return getRegExp().getRex();
	}
	bool is64bitDisp() const { return mode_ == M_64bitDisp; } // for moffset
	bool isBroadcast() const { return broadcast_; }
	const Label* getLabel() const { return label_; }
	bool operator==(const Address& rhs) const
	{
		return getBit() == rhs.getBit() && e_ == rhs.e_ && label_ == rhs.label_ && mode_ == rhs.mode_ && broadcast_ == rhs.broadcast_;
	}
	bool operator!=(const Address& rhs) const { return !operator==(rhs); }
	bool isVsib() const { return e_.isVsib(); }
private:
	RegExp e_;
	const Label* label_;
	Mode mode_;
	bool broadcast_;
};

inline const Address& Operand::getAddress() const
{
	assert(isMEM());
	return static_cast<const Address&>(*this);
}

inline bool Operand::operator==(const Operand& rhs) const
{
	if (isMEM() && rhs.isMEM()) return this->getAddress() == rhs.getAddress();
	return isEqualIfNotInherited(rhs);
}

class AddressFrame {
	void operator=(const AddressFrame&);
	AddressFrame(const AddressFrame&);
public:
	const uint32 bit_;
	const bool broadcast_;
	explicit AddressFrame(uint32 bit, bool broadcast = false) : bit_(bit), broadcast_(broadcast) { }
	Address operator[](const RegExp& e) const
	{
		return Address(bit_, broadcast_, e);
	}
	Address operator[](const void *disp) const
	{
		return Address(bit_, broadcast_, RegExp(reinterpret_cast<size_t>(disp)));
	}
#ifdef XBYAK64
	Address operator[](uint64 disp) const { return Address(disp); }
	Address operator[](const RegRip& addr) const { return Address(bit_, broadcast_, addr); }
#endif
};

struct JmpLabel {
	size_t endOfJmp; /* offset from top to the end address of jmp */
	int jmpSize;
	inner::LabelMode mode;
	size_t disp; // disp for [rip + disp]
	explicit JmpLabel(size_t endOfJmp = 0, int jmpSize = 0, inner::LabelMode mode = inner::LasIs, size_t disp = 0)
		: endOfJmp(endOfJmp), jmpSize(jmpSize), mode(mode), disp(disp)
	{
	}
};

class LabelManager;

class Label {
	mutable LabelManager *mgr;
	mutable int id;
	friend class LabelManager;
public:
	Label() : mgr(0), id(0) {}
	Label(const Label& rhs);
	Label& operator=(const Label& rhs);
	~Label();
	void clear() { mgr = 0; id = 0; }
	int getId() const { return id; }
	const uint8 *getAddress() const;

	// backward compatibility
	static inline std::string toStr(int num)
	{
		char buf[16];
#if defined(_MSC_VER) && (_MSC_VER < 1900)
		_snprintf_s
#else
		snprintf
#endif
		(buf, sizeof(buf), ".%08x", num);
		return buf;
	}
};

class LabelManager {
	// for string label
	struct SlabelVal {
		size_t offset;
		SlabelVal(size_t offset) : offset(offset) {}
	};
	typedef XBYAK_STD_UNORDERED_MAP<std::string, SlabelVal> SlabelDefList;
	typedef XBYAK_STD_UNORDERED_MULTIMAP<std::string, const JmpLabel> SlabelUndefList;
	struct SlabelState {
		SlabelDefList defList;
		SlabelUndefList undefList;
	};
	typedef std::list<SlabelState> StateList;
	// for Label class
	struct ClabelVal {
		ClabelVal(size_t offset = 0) : offset(offset), refCount(1) {}
		size_t offset;
		int refCount;
	};
	typedef XBYAK_STD_UNORDERED_MAP<int, ClabelVal> ClabelDefList;
	typedef XBYAK_STD_UNORDERED_MULTIMAP<int, const JmpLabel> ClabelUndefList;
	typedef XBYAK_STD_UNORDERED_SET<Label*> LabelPtrList;

	CodeArray *base_;
	// global : stateList_.front(), local : stateList_.back()
	StateList stateList_;
	mutable int labelId_;
	ClabelDefList clabelDefList_;
	ClabelUndefList clabelUndefList_;
	LabelPtrList labelPtrList_;

	int getId(const Label& label) const;
	template<class DefList, class UndefList, class T>
	void define_inner(DefList& defList, UndefList& undefList, const T& labelId, size_t addrOffset);
	template<class DefList, class T>
	bool getOffset_inner(const DefList& defList, size_t *offset, const T& label) const;
	friend class Label;
	void incRefCount(int id, Label *label);
	void decRefCount(int id, Label *label);
	template<class T>
	bool hasUndefinedLabel_inner(const T& list) const;
	// detach all labels linked to LabelManager
	void resetLabelPtrList();
public:
	LabelManager()
	{
		reset();
	}
	~LabelManager()
	{
		resetLabelPtrList();
	}
	void reset();
	void enterLocal();
	void leaveLocal();
	void set(CodeArray *base) { base_ = base; }
	void defineSlabel(std::string label);
	void defineClabel(Label& label);
	void assign(Label& dst, const Label& src);
	bool getOffset(size_t *offset, std::string& label) const;
	bool getOffset(size_t *offset, const Label& label) const;
	void addUndefinedLabel(const std::string& label, const JmpLabel& jmp);
	void addUndefinedLabel(const Label& label, const JmpLabel& jmp);
	bool hasUndefSlabel() const;
	bool hasUndefClabel() const { return hasUndefinedLabel_inner(clabelUndefList_); }
	const uint8 *getCode() const { return base_->getCode(); }
	bool isReady() const { return !base_->isAutoGrow() || base_->isCalledCalcJmpAddress(); }
};

class CodeGenerator : public CodeArray {
public:
	enum LabelType {
		T_SHORT,
		T_NEAR,
		T_AUTO // T_SHORT if possible
	};
private:
	CodeGenerator operator=(const CodeGenerator&); // don't call
#ifdef XBYAK64
	enum { i32e = 32 | 64, BIT = 64 };
	static const uint64 dummyAddr = uint64(0x1122334455667788ull);
	typedef Reg64 NativeReg;
#else
	enum { i32e = 32, BIT = 32 };
	static const size_t dummyAddr = 0x12345678;
	typedef Reg32 NativeReg;
#endif
	// (XMM, XMM|MEM)
	static bool isXMM_XMMorMEM(const Operand& op1, const Operand& op2);
	// (MMX, MMX|MEM) or (XMM, XMM|MEM)
	static bool isXMMorMMX_MEM(const Operand& op1, const Operand& op2);
	// (XMM, MMX|MEM)
	static bool isXMM_MMXorMEM(const Operand& op1, const Operand& op2);
	// (MMX, XMM|MEM)
	static bool isMMX_XMMorMEM(const Operand& op1, const Operand& op2);
	// (XMM, REG32|MEM)
	static bool isXMM_REG32orMEM(const Operand& op1, const Operand& op2);
	// (REG32, XMM|MEM)
	static bool isREG32_XMMorMEM(const Operand& op1, const Operand& op2);
	// (REG32, REG32|MEM)
	static bool isREG32_REG32orMEM(const Operand& op1, const Operand& op2);
	void rex(const Operand& op1, const Operand& op2 = Operand());
	enum AVXtype {
		// low 3 bit
		T_N1 = 1,
		T_N2 = 2,
		T_N4 = 3,
		T_N8 = 4,
		T_N16 = 5,
		T_N32 = 6,
		T_NX_MASK = 7,
		//
		T_N_VL = 1 << 3, // N * (1, 2, 4) for VL
		T_DUP = 1 << 4, // N = (8, 32, 64)
		T_66 = 1 << 5,
		T_F3 = 1 << 6,
		T_F2 = 1 << 7,
		T_0F = 1 << 8,
		T_0F38 = 1 << 9,
		T_0F3A = 1 << 10,
		T_L0 = 1 << 11,
		T_L1 = 1 << 12,
		T_W0 = 1 << 13,
		T_W1 = 1 << 14,
		T_EW0 = 1 << 15,
		T_EW1 = 1 << 16,
		T_YMM = 1 << 17, // support YMM, ZMM
		T_EVEX = 1 << 18,
		T_ER_X = 1 << 19, // xmm{er}
		T_ER_Y = 1 << 20, // ymm{er}
		T_ER_Z = 1 << 21, // zmm{er}
		T_SAE_X = 1 << 22, // xmm{sae}
		T_SAE_Y = 1 << 23, // ymm{sae}
		T_SAE_Z = 1 << 24, // zmm{sae}
		T_MUST_EVEX = 1 << 25, // contains T_EVEX
		T_B32 = 1 << 26, // m32bcst
		T_B64 = 1 << 27, // m64bcst
		T_M_K = 1 << 28, // mem{k}
		T_VSIB = 1 << 29,
		T_MEM_EVEX = 1 << 30, // use evex if mem
		T_XXX
	};
	void vex(const Reg& reg, const Reg& base, const Operand *v, int type, int code, bool x = false);
	void verifySAE(const Reg& r, int type) const;
	void verifyER(const Reg& r, int type) const;
	// (a, b, c) contains non zero two or three values then err
	int verifyDuplicate(int a, int b, int c, int err);
	int evex(const Reg& reg, const Reg& base, const Operand *v, int type, int code, bool x = false, bool b = false, int aaa = 0, uint32 VL = 0, bool Hi16Vidx = false);
	void setModRM(int mod, int r1, int r2);
	void setSIB(const RegExp& e, int reg, int disp8N = 0);
	LabelManager labelMgr_;
	bool isInDisp16(uint32 x) const { return 0xFFFF8000 <= x || x <= 0x7FFF; }
	void opModR(const Reg& reg1, const Reg& reg2, int code0, int code1 = NONE, int code2 = NONE);
	void opModM(const Address& addr, const Reg& reg, int code0, int code1 = NONE, int code2 = NONE, int immSize = 0);
	void opLoadSeg(const Address& addr, const Reg& reg, int code0, int code1 = NONE);
	void opMIB(const Address& addr, const Reg& reg, int code0, int code1);
	void makeJmp(uint32 disp, LabelType type, uint8 shortCode, uint8 longCode, uint8 longPref);
	bool isNEAR(LabelType type) const { return type == T_NEAR || (type == T_AUTO && isDefaultJmpNEAR_); }
	template<class T>
	void opJmp(T& label, LabelType type, uint8 shortCode, uint8 longCode, uint8 longPref);
	void opJmpAbs(const void *addr, LabelType type, uint8 shortCode, uint8 longCode, uint8 longPref = 0);
	// reg is reg field of ModRM
	// immSize is the size for immediate value
	// disp8N = 0(normal), disp8N = 1(force disp32), disp8N = {2, 4, 8} ; compressed displacement
	void opAddr(const Address &addr, int reg, int immSize = 0, int disp8N = 0, bool permitVisb = false);
	/* preCode is for SSSE3/SSE4 */
	void opGen(const Operand& reg, const Operand& op, int code, int pref, bool isValid(const Operand&, const Operand&), int imm8 = NONE, int preCode = NONE);
	void opMMX_IMM(const Mmx& mmx, int imm8, int code, int ext);
	void opMMX(const Mmx& mmx, const Operand& op, int code, int pref = 0x66, int imm8 = NONE, int preCode = NONE);
	void opMovXMM(const Operand& op1, const Operand& op2, int code, int pref);
	void opExt(const Operand& op, const Mmx& mmx, int code, int imm, bool hasMMX2 = false);
	void opR_ModM(const Operand& op, int bit, int ext, int code0, int code1 = NONE, int code2 = NONE, bool disableRex = false, int immSize = 0);
	void opShift(const Operand& op, int imm, int ext);
	void opShift(const Operand& op, const Reg8& _cl, int ext);
	void opModRM(const Operand& op1, const Operand& op2, bool condR, bool condM, int code0, int code1 = NONE, int code2 = NONE, int immSize = 0);
	void opShxd(const Operand& op, const Reg& reg, uint8 imm, int code, const Reg8 *_cl = 0);
	// (REG, REG|MEM), (MEM, REG)
	void opRM_RM(const Operand& op1, const Operand& op2, int code);
	// (REG|MEM, IMM)
	void opRM_I(const Operand& op, uint32 imm, int code, int ext);
	void opIncDec(const Operand& op, int code, int ext);
	void opPushPop(const Operand& op, int code, int ext, int alt);
	void verifyMemHasSize(const Operand& op) const;
	/*
		mov(r, imm) = db(imm, mov_imm(r, imm))
	*/
	int mov_imm(const Reg& reg, uint64 imm);
	template<class T>
	void putL_inner(T& label, bool relative = false, size_t disp = 0);
	void opMovxx(const Reg& reg, const Operand& op, uint8 code);
	void opFpuMem(const Address& addr, uint8 m16, uint8 m32, uint8 m64, uint8 ext, uint8 m64ext);
	// use code1 if reg1 == st0
	// use code2 if reg1 != st0 && reg2 == st0
	void opFpuFpu(const Fpu& reg1, const Fpu& reg2, uint32 code1, uint32 code2);
	void opFpu(const Fpu& reg, uint8 code1, uint8 code2);
	void opVex(const Reg& r, const Operand *p1, const Operand& op2, int type, int code, int imm8 = NONE);
	// (r, r, r/m) if isR_R_RM
	// (r, r/m, r)
	void opGpr(const Reg32e& r, const Operand& op1, const Operand& op2, int type, uint8 code, bool isR_R_RM, int imm8 = NONE);
	void opAVX_X_X_XM(const Xmm& x1, const Operand& op1, const Operand& op2, int type, int code0, int imm8 = NONE);
	void opAVX_K_X_XM(const Opmask& k, const Xmm& x2, const Operand& op3, int type, int code0, int imm8 = NONE);
	// (x, x/m), (y, x/m256), (z, y/m)
	void checkCvt1(const Operand& x, const Operand& op) const;
	// (x, x/m), (x, y/m256), (y, z/m)
	void checkCvt2(const Xmm& x, const Operand& op) const;
	void opCvt2(const Xmm& x, const Operand& op, int type, int code);
	void opCvt3(const Xmm& x1, const Xmm& x2, const Operand& op, int type, int type64, int type32, uint8 code);
	const Xmm& cvtIdx0(const Operand& x) const
	{
		return x.isZMM() ? zm0 : x.isYMM() ? ym0 : xm0;
	}
	// support (x, x/m, imm), (y, y/m, imm)
	void opAVX_X_XM_IMM(const Xmm& x, const Operand& op, int type, int code, int imm8 = NONE);
	// QQQ:need to refactor
	void opSp1(const Reg& reg, const Operand& op, uint8 pref, uint8 code0, uint8 code1);
	void opGather(const Xmm& x1, const Address& addr, const Xmm& x2, int type, uint8 code, int mode);
	enum {
		xx_yy_zz = 0,
		xx_yx_zy = 1,
		xx_xy_yz = 2
	};
	void checkGather2(const Xmm& x1, const Reg& x2, int mode) const;
	void opGather2(const Xmm& x, const Address& addr, int type, uint8 code, int mode);
	/*
		xx_xy_yz ; mode = true
		xx_xy_xz ; mode = false
	*/
	void opVmov(const Operand& op, const Xmm& x, int type, uint8 code, bool mode);
	void opGatherFetch(const Address& addr, const Xmm& x, int type, uint8 code, Operand::Kind kind);
	void opInOut(const Reg& a, const Reg& d, uint8 code);
	void opInOut(const Reg& a, uint8 code, uint8 v);
#ifdef XBYAK64
	void opAMX(const Tmm& t1, const Address& addr, int type, int code0);
#endif
public:
	unsigned int getVersion() const { return VERSION; }
	using CodeArray::db;
	const Mmx mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	const Xmm xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	const Ymm ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymm6, ymm7;
	const Zmm zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7;
	const Xmm &xm0, &xm1, &xm2, &xm3, &xm4, &xm5, &xm6, &xm7;
	const Ymm &ym0, &ym1, &ym2, &ym3, &ym4, &ym5, &ym6, &ym7;
	const Zmm &zm0, &zm1, &zm2, &zm3, &zm4, &zm5, &zm6, &zm7;
	const Reg32 eax, ecx, edx, ebx, esp, ebp, esi, edi;
	const Reg16 ax, cx, dx, bx, sp, bp, si, di;
	const Reg8 al, cl, dl, bl, ah, ch, dh, bh;
	const AddressFrame ptr, byte, word, dword, qword, xword, yword, zword; // xword is same as oword of NASM
	const AddressFrame ptr_b, xword_b, yword_b, zword_b; // broadcast such as {1to2}, {1to4}, {1to8}, {1to16}, {b}
	const Fpu st0, st1, st2, st3, st4, st5, st6, st7;
	const Opmask k0, k1, k2, k3, k4, k5, k6, k7;
	const BoundsReg bnd0, bnd1, bnd2, bnd3;
	const EvexModifierRounding T_sae, T_rn_sae, T_rd_sae, T_ru_sae, T_rz_sae; // {sae}, {rn-sae}, {rd-sae}, {ru-sae}, {rz-sae}
	const EvexModifierZero T_z; // {z}
#ifdef XBYAK64
	const Reg64 rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15;
	const Reg32 r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d;
	const Reg16 r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w;
	const Reg8 r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b;
	const Reg8 spl, bpl, sil, dil;
	const Xmm xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
	const Xmm xmm16, xmm17, xmm18, xmm19, xmm20, xmm21, xmm22, xmm23;
	const Xmm xmm24, xmm25, xmm26, xmm27, xmm28, xmm29, xmm30, xmm31;
	const Ymm ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15;
	const Ymm ymm16, ymm17, ymm18, ymm19, ymm20, ymm21, ymm22, ymm23;
	const Ymm ymm24, ymm25, ymm26, ymm27, ymm28, ymm29, ymm30, ymm31;
	const Zmm zmm8, zmm9, zmm10, zmm11, zmm12, zmm13, zmm14, zmm15;
	const Zmm zmm16, zmm17, zmm18, zmm19, zmm20, zmm21, zmm22, zmm23;
	const Zmm zmm24, zmm25, zmm26, zmm27, zmm28, zmm29, zmm30, zmm31;
	const Tmm tmm0, tmm1, tmm2, tmm3, tmm4, tmm5, tmm6, tmm7;
	const Xmm &xm8, &xm9, &xm10, &xm11, &xm12, &xm13, &xm14, &xm15; // for my convenience
	const Xmm &xm16, &xm17, &xm18, &xm19, &xm20, &xm21, &xm22, &xm23;
	const Xmm &xm24, &xm25, &xm26, &xm27, &xm28, &xm29, &xm30, &xm31;
	const Ymm &ym8, &ym9, &ym10, &ym11, &ym12, &ym13, &ym14, &ym15;
	const Ymm &ym16, &ym17, &ym18, &ym19, &ym20, &ym21, &ym22, &ym23;
	const Ymm &ym24, &ym25, &ym26, &ym27, &ym28, &ym29, &ym30, &ym31;
	const Zmm &zm8, &zm9, &zm10, &zm11, &zm12, &zm13, &zm14, &zm15;
	const Zmm &zm16, &zm17, &zm18, &zm19, &zm20, &zm21, &zm22, &zm23;
	const Zmm &zm24, &zm25, &zm26, &zm27, &zm28, &zm29, &zm30, &zm31;
	const RegRip rip;
#endif
#ifndef XBYAK_DISABLE_SEGMENT
	const Segment es, cs, ss, ds, fs, gs;
#endif
private:
	bool isDefaultJmpNEAR_;
public:
	void L(const std::string& label);
	void L(Label& label);
	Label L();
	void inLocalLabel();
	void outLocalLabel();
	/*
		assign src to dst
		require
		dst : does not used by L()
		src : used by L()
	*/
	void assignL(Label& dst, const Label& src);
	/*
		put address of label to buffer
		@note the put size is 4(32-bit), 8(64-bit)
	*/
	void putL(std::string label);
	void putL(const Label& label);

	// set default type of `jmp` of undefined label to T_NEAR
	void setDefaultJmpNEAR(bool isNear);
	void jmp(const Operand& op);
	void jmp(std::string label, LabelType type = T_AUTO);
	void jmp(const char *label, LabelType type = T_AUTO);
	void jmp(const Label& label, LabelType type = T_AUTO);
	void jmp(const void *addr, LabelType type = T_AUTO);

	void call(const Operand& op);
	// call(string label), not const std::string&
	void call(std::string label);
	void call(const char *label);
	void call(const Label& label);
	// call(function pointer)
#ifdef XBYAK_VARIADIC_TEMPLATE
	template<class Ret, class... Params>
	void call(Ret(*func)(Params...)) { call(reinterpret_cast<const void*>(func)); }
#endif
	void call(const void *addr);

	void test(const Operand& op, const Reg& reg);
	void test(const Operand& op, uint32 imm);
	void imul(const Reg& reg, const Operand& op);
	void imul(const Reg& reg, const Operand& op, int imm);
	void push(const Operand& op);
	void pop(const Operand& op);
	void push(const AddressFrame& af, uint32 imm);
	/* use "push(word, 4)" if you want "push word 4" */
	void push(uint32 imm);
	void mov(const Operand& reg1, const Operand& reg2);
	void mov(const Operand& op, uint64 imm);
	void mov(const NativeReg& reg, const char *label);
	void mov(const NativeReg& reg, const Label& label);
	void xchg(const Operand& op1, const Operand& op2);

#ifndef XBYAK_DISABLE_SEGMENT
	void push(const Segment& seg);
	void pop(const Segment& seg);
	void putSeg(const Segment& seg);
	void mov(const Operand& op, const Segment& seg);
	void mov(const Segment& seg, const Operand& op);
#endif

	enum { NONE = 256 };
	// constructor
	CodeGenerator(size_t maxSize = DEFAULT_MAX_CODE_SIZE, void *userPtr = 0, Allocator *allocator = 0)
		: CodeArray(maxSize, userPtr, allocator)
		, mm0(0), mm1(1), mm2(2), mm3(3), mm4(4), mm5(5), mm6(6), mm7(7)
		, xmm0(0), xmm1(1), xmm2(2), xmm3(3), xmm4(4), xmm5(5), xmm6(6), xmm7(7)
		, ymm0(0), ymm1(1), ymm2(2), ymm3(3), ymm4(4), ymm5(5), ymm6(6), ymm7(7)
		, zmm0(0), zmm1(1), zmm2(2), zmm3(3), zmm4(4), zmm5(5), zmm6(6), zmm7(7)
		// for my convenience
		, xm0(xmm0), xm1(xmm1), xm2(xmm2), xm3(xmm3), xm4(xmm4), xm5(xmm5), xm6(xmm6), xm7(xmm7)
		, ym0(ymm0), ym1(ymm1), ym2(ymm2), ym3(ymm3), ym4(ymm4), ym5(ymm5), ym6(ymm6), ym7(ymm7)
		, zm0(zmm0), zm1(zmm1), zm2(zmm2), zm3(zmm3), zm4(zmm4), zm5(zmm5), zm6(zmm6), zm7(zmm7)

		, eax(Operand::EAX), ecx(Operand::ECX), edx(Operand::EDX), ebx(Operand::EBX), esp(Operand::ESP), ebp(Operand::EBP), esi(Operand::ESI), edi(Operand::EDI)
		, ax(Operand::AX), cx(Operand::CX), dx(Operand::DX), bx(Operand::BX), sp(Operand::SP), bp(Operand::BP), si(Operand::SI), di(Operand::DI)
		, al(Operand::AL), cl(Operand::CL), dl(Operand::DL), bl(Operand::BL), ah(Operand::AH), ch(Operand::CH), dh(Operand::DH), bh(Operand::BH)
		, ptr(0), byte(8), word(16), dword(32), qword(64), xword(128), yword(256), zword(512)
		, ptr_b(0, true), xword_b(128, true), yword_b(256, true), zword_b(512, true)
		, st0(0), st1(1), st2(2), st3(3), st4(4), st5(5), st6(6), st7(7)
		, k0(0), k1(1), k2(2), k3(3), k4(4), k5(5), k6(6), k7(7)
		, bnd0(0), bnd1(1), bnd2(2), bnd3(3)
		, T_sae(EvexModifierRounding::T_SAE), T_rn_sae(EvexModifierRounding::T_RN_SAE), T_rd_sae(EvexModifierRounding::T_RD_SAE), T_ru_sae(EvexModifierRounding::T_RU_SAE), T_rz_sae(EvexModifierRounding::T_RZ_SAE)
		, T_z()
#ifdef XBYAK64
		, rax(Operand::RAX), rcx(Operand::RCX), rdx(Operand::RDX), rbx(Operand::RBX), rsp(Operand::RSP), rbp(Operand::RBP), rsi(Operand::RSI), rdi(Operand::RDI), r8(Operand::R8), r9(Operand::R9), r10(Operand::R10), r11(Operand::R11), r12(Operand::R12), r13(Operand::R13), r14(Operand::R14), r15(Operand::R15)
		, r8d(8), r9d(9), r10d(10), r11d(11), r12d(12), r13d(13), r14d(14), r15d(15)
		, r8w(8), r9w(9), r10w(10), r11w(11), r12w(12), r13w(13), r14w(14), r15w(15)
		, r8b(8), r9b(9), r10b(10), r11b(11), r12b(12), r13b(13), r14b(14), r15b(15)
		, spl(Operand::SPL, true), bpl(Operand::BPL, true), sil(Operand::SIL, true), dil(Operand::DIL, true)
		, xmm8(8), xmm9(9), xmm10(10), xmm11(11), xmm12(12), xmm13(13), xmm14(14), xmm15(15)
		, xmm16(16), xmm17(17), xmm18(18), xmm19(19), xmm20(20), xmm21(21), xmm22(22), xmm23(23)
		, xmm24(24), xmm25(25), xmm26(26), xmm27(27), xmm28(28), xmm29(29), xmm30(30), xmm31(31)
		, ymm8(8), ymm9(9), ymm10(10), ymm11(11), ymm12(12), ymm13(13), ymm14(14), ymm15(15)
		, ymm16(16), ymm17(17), ymm18(18), ymm19(19), ymm20(20), ymm21(21), ymm22(22), ymm23(23)
		, ymm24(24), ymm25(25), ymm26(26), ymm27(27), ymm28(28), ymm29(29), ymm30(30), ymm31(31)
		, zmm8(8), zmm9(9), zmm10(10), zmm11(11), zmm12(12), zmm13(13), zmm14(14), zmm15(15)
		, zmm16(16), zmm17(17), zmm18(18), zmm19(19), zmm20(20), zmm21(21), zmm22(22), zmm23(23)
		, zmm24(24), zmm25(25), zmm26(26), zmm27(27), zmm28(28), zmm29(29), zmm30(30), zmm31(31)
		, tmm0(0), tmm1(1), tmm2(2), tmm3(3), tmm4(4), tmm5(5), tmm6(6), tmm7(7)
		// for my convenience
		, xm8(xmm8), xm9(xmm9), xm10(xmm10), xm11(xmm11), xm12(xmm12), xm13(xmm13), xm14(xmm14), xm15(xmm15)
		, xm16(xmm16), xm17(xmm17), xm18(xmm18), xm19(xmm19), xm20(xmm20), xm21(xmm21), xm22(xmm22), xm23(xmm23)
		, xm24(xmm24), xm25(xmm25), xm26(xmm26), xm27(xmm27), xm28(xmm28), xm29(xmm29), xm30(xmm30), xm31(xmm31)
		, ym8(ymm8), ym9(ymm9), ym10(ymm10), ym11(ymm11), ym12(ymm12), ym13(ymm13), ym14(ymm14), ym15(ymm15)
		, ym16(ymm16), ym17(ymm17), ym18(ymm18), ym19(ymm19), ym20(ymm20), ym21(ymm21), ym22(ymm22), ym23(ymm23)
		, ym24(ymm24), ym25(ymm25), ym26(ymm26), ym27(ymm27), ym28(ymm28), ym29(ymm29), ym30(ymm30), ym31(ymm31)
		, zm8(zmm8), zm9(zmm9), zm10(zmm10), zm11(zmm11), zm12(zmm12), zm13(zmm13), zm14(zmm14), zm15(zmm15)
		, zm16(zmm16), zm17(zmm17), zm18(zmm18), zm19(zmm19), zm20(zmm20), zm21(zmm21), zm22(zmm22), zm23(zmm23)
		, zm24(zmm24), zm25(zmm25), zm26(zmm26), zm27(zmm27), zm28(zmm28), zm29(zmm29), zm30(zmm30), zm31(zmm31)
		, rip()
#endif
#ifndef XBYAK_DISABLE_SEGMENT
		, es(Segment::es), cs(Segment::cs), ss(Segment::ss), ds(Segment::ds), fs(Segment::fs), gs(Segment::gs)
#endif
		, isDefaultJmpNEAR_(false)
	{
		labelMgr_.set(this);
	}
	void reset()
	{
		resetSize();
		labelMgr_.reset();
		labelMgr_.set(this);
	}
	bool hasUndefinedLabel() const { return labelMgr_.hasUndefSlabel() || labelMgr_.hasUndefClabel(); }
	/*
		MUST call ready() to complete generating code if you use AutoGrow mode.
		It is not necessary for the other mode if hasUndefinedLabel() is true.
	*/
	void ready(ProtectMode mode = PROTECT_RWE)
	{
		if (hasUndefinedLabel()) XBYAK_THROW(ERR_LABEL_IS_NOT_FOUND)
		if (isAutoGrow()) {
			calcJmpAddress();
			if (useProtect()) setProtectMode(mode);
		}
	}
	// set read/exec
	void readyRE() { return ready(PROTECT_RE); }
#ifdef XBYAK_TEST
	void dump(bool doClear = true)
	{
		CodeArray::dump();
		if (doClear) size_ = 0;
	}
#endif

#ifdef XBYAK_UNDEF_JNL
	#undef jnl
#endif

	/*
		use single byte nop if useMultiByteNop = false
	*/
	void nop(size_t size = 1, bool useMultiByteNop = true);

#ifndef XBYAK_DONT_READ_LIST
#if XBYAK_SPLIT == 1
#include "xbyak_mnemonic_decl_only.h"
#else
#include "xbyak_mnemonic.h"
#endif
	/*
		use single byte nop if useMultiByteNop = false
	*/
	void align(size_t x = 16, bool useMultiByteNop = true);
#endif
};

namespace util {
static const Mmx mm0(0), mm1(1), mm2(2), mm3(3), mm4(4), mm5(5), mm6(6), mm7(7);
static const Xmm xmm0(0), xmm1(1), xmm2(2), xmm3(3), xmm4(4), xmm5(5), xmm6(6), xmm7(7);
static const Ymm ymm0(0), ymm1(1), ymm2(2), ymm3(3), ymm4(4), ymm5(5), ymm6(6), ymm7(7);
static const Zmm zmm0(0), zmm1(1), zmm2(2), zmm3(3), zmm4(4), zmm5(5), zmm6(6), zmm7(7);
static const Reg32 eax(Operand::EAX), ecx(Operand::ECX), edx(Operand::EDX), ebx(Operand::EBX), esp(Operand::ESP), ebp(Operand::EBP), esi(Operand::ESI), edi(Operand::EDI);
static const Reg16 ax(Operand::AX), cx(Operand::CX), dx(Operand::DX), bx(Operand::BX), sp(Operand::SP), bp(Operand::BP), si(Operand::SI), di(Operand::DI);
static const Reg8 al(Operand::AL), cl(Operand::CL), dl(Operand::DL), bl(Operand::BL), ah(Operand::AH), ch(Operand::CH), dh(Operand::DH), bh(Operand::BH);
static const AddressFrame ptr(0), byte(8), word(16), dword(32), qword(64), xword(128), yword(256), zword(512);
static const AddressFrame ptr_b(0, true), xword_b(128, true), yword_b(256, true), zword_b(512, true);
static const Fpu st0(0), st1(1), st2(2), st3(3), st4(4), st5(5), st6(6), st7(7);
static const Opmask k0(0), k1(1), k2(2), k3(3), k4(4), k5(5), k6(6), k7(7);
static const BoundsReg bnd0(0), bnd1(1), bnd2(2), bnd3(3);
static const EvexModifierRounding T_sae(EvexModifierRounding::T_SAE), T_rn_sae(EvexModifierRounding::T_RN_SAE), T_rd_sae(EvexModifierRounding::T_RD_SAE), T_ru_sae(EvexModifierRounding::T_RU_SAE), T_rz_sae(EvexModifierRounding::T_RZ_SAE);
static const EvexModifierZero T_z;
#ifdef XBYAK64
static const Reg64 rax(Operand::RAX), rcx(Operand::RCX), rdx(Operand::RDX), rbx(Operand::RBX), rsp(Operand::RSP), rbp(Operand::RBP), rsi(Operand::RSI), rdi(Operand::RDI), r8(Operand::R8), r9(Operand::R9), r10(Operand::R10), r11(Operand::R11), r12(Operand::R12), r13(Operand::R13), r14(Operand::R14), r15(Operand::R15);
static const Reg32 r8d(8), r9d(9), r10d(10), r11d(11), r12d(12), r13d(13), r14d(14), r15d(15);
static const Reg16 r8w(8), r9w(9), r10w(10), r11w(11), r12w(12), r13w(13), r14w(14), r15w(15);
static const Reg8 r8b(8), r9b(9), r10b(10), r11b(11), r12b(12), r13b(13), r14b(14), r15b(15), spl(Operand::SPL, true), bpl(Operand::BPL, true), sil(Operand::SIL, true), dil(Operand::DIL, true);
static const Xmm xmm8(8), xmm9(9), xmm10(10), xmm11(11), xmm12(12), xmm13(13), xmm14(14), xmm15(15);
static const Xmm xmm16(16), xmm17(17), xmm18(18), xmm19(19), xmm20(20), xmm21(21), xmm22(22), xmm23(23);
static const Xmm xmm24(24), xmm25(25), xmm26(26), xmm27(27), xmm28(28), xmm29(29), xmm30(30), xmm31(31);
static const Ymm ymm8(8), ymm9(9), ymm10(10), ymm11(11), ymm12(12), ymm13(13), ymm14(14), ymm15(15);
static const Ymm ymm16(16), ymm17(17), ymm18(18), ymm19(19), ymm20(20), ymm21(21), ymm22(22), ymm23(23);
static const Ymm ymm24(24), ymm25(25), ymm26(26), ymm27(27), ymm28(28), ymm29(29), ymm30(30), ymm31(31);
static const Zmm zmm8(8), zmm9(9), zmm10(10), zmm11(11), zmm12(12), zmm13(13), zmm14(14), zmm15(15);
static const Zmm zmm16(16), zmm17(17), zmm18(18), zmm19(19), zmm20(20), zmm21(21), zmm22(22), zmm23(23);
static const Zmm zmm24(24), zmm25(25), zmm26(26), zmm27(27), zmm28(28), zmm29(29), zmm30(30), zmm31(31);
static const Tmm tmm0(0), tmm1(1), tmm2(2), tmm3(3), tmm4(4), tmm5(5), tmm6(6), tmm7(7);
static const RegRip rip;
#endif
#ifndef XBYAK_DISABLE_SEGMENT
static const Segment es(Segment::es), cs(Segment::cs), ss(Segment::ss), ds(Segment::ds), fs(Segment::fs), gs(Segment::gs);
#endif
} // util

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

} // end of namespace

#endif // XBYAK_XBYAK_H_
