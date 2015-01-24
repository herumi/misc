#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

uint64_t get1(const uint8_t *p)
{
	return *p;
}
uint64_t get2(const uint8_t *p)
{
	uint16_t r;
	memcpy(&r, p, 2);
	return r;
}
uint64_t get3(const uint8_t *p)
{
	return get2(p) | (get1(p + 2) << 16);
}
uint64_t get4(const uint8_t *p)
{
	uint32_t r;
	memcpy(&r, p, 4);
	return r;
}
uint64_t get5(const uint8_t *p)
{
	return get4(p) | (get1(p + 4) << 32);
}
uint64_t get6(const uint8_t *p)
{
	return get4(p) | (get2(p + 4) << 32);
}
uint64_t get7(const uint8_t *p)
{
	return get4(p) | (get3(p + 4) << 32);
}
uint64_t get8(const uint8_t *p)
{
	uint64_t r;
	memcpy(&r, p, 8);
	return r;
}
bool is_same1(const uint8_t *p, const uint8_t *q) { return get1(p) ^ get1(q); }
bool is_same2(const uint8_t *p, const uint8_t *q) { return get2(p) ^ get2(q); }
bool is_same3(const uint8_t *p, const uint8_t *q) { return get3(p) ^ get3(q); }
bool is_same4(const uint8_t *p, const uint8_t *q) { return get4(p) ^ get4(q); }
bool is_same5(const uint8_t *p, const uint8_t *q) { return get5(p) ^ get5(q); }
bool is_same6(const uint8_t *p, const uint8_t *q) { return get6(p) ^ get6(q); }
bool is_same7(const uint8_t *p, const uint8_t *q) { return get7(p) ^ get7(q); }
bool is_same8(const uint8_t *p, const uint8_t *q) { return get8(p) ^ get8(q); }
bool is_same9(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get1(p + 8) ^ get1(q + 8)); }
bool is_same10(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get2(p + 8) ^ get2(q + 8)); }
bool is_same11(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get3(p + 8) ^ get3(q + 8)); }
bool is_same12(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get4(p + 8) ^ get4(q + 8)); }
bool is_same13(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get5(p + 8) ^ get5(q + 8)); }
bool is_same14(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get6(p + 8) ^ get6(q + 8)); }
bool is_same15(const uint8_t *p, const uint8_t *q) { return (get8(p) ^ get8(q)) | (get7(p + 8) ^ get7(q + 8)); }
bool is_same16(const uint8_t *p, const uint8_t *q)
{
	__m128i x = _mm_loadu_si128((const __m128i*)p);
	__m128i y = _mm_loadu_si128((const __m128i*)q);
	return _mm_testc_si128(x, y) != 0;
}
bool is_same17(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same1(p + 16, q + 16) : false; }
bool is_same18(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same2(p + 16, q + 16) : false; }
bool is_same19(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same3(p + 16, q + 16) : false; }
bool is_same20(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same4(p + 16, q + 16) : false; }
bool is_same21(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same5(p + 16, q + 16) : false; }
bool is_same22(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same6(p + 16, q + 16) : false; }
bool is_same23(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same7(p + 16, q + 16) : false; }
bool is_same24(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same8(p + 16, q + 16) : false; }
bool is_same25(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same9(p + 16, q + 16) : false; }
bool is_same26(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same10(p + 16, q + 16) : false; }
bool is_same27(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same11(p + 16, q + 16) : false; }
bool is_same28(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same12(p + 16, q + 16) : false; }
bool is_same29(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same13(p + 16, q + 16) : false; }
bool is_same30(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same14(p + 16, q + 16) : false; }
bool is_same31(const uint8_t *p, const uint8_t *q) { return is_same16(p, q) ? is_same15(p + 16, q + 16) : false; }
bool is_same32(const uint8_t *p, const uint8_t *q)
{
	__m256i x = _mm256_loadu_si256((const __m256i*)p);
	__m256i y = _mm256_loadu_si256((const __m256i*)q);
	return _mm256_testc_si256(x, y) != 0;
}

#if 0

#define is_same(p1, p2, n) \
({ \
	const uint8_t *p = (const uint8_t *)p1; \
	const uint8_t *q = (const uint8_t *)p2; \
	uint64_t ret; \
	if (__builtin_constant_p(n)) { \
		switch (n) { \
		case 1: ret = is_same1(p, q); break; \
		case 2: ret = is_same2(p, q); break; \
		case 3: ret = is_same3(p, q); break; \
		case 4: ret = is_same4(p, q); break; \
		case 5: ret = is_same5(p, q); break; \
		case 6: ret = is_same6(p, q); break; \
		case 7: ret = is_same7(p, q); break; \
		case 8: ret = is_same8(p, q); break; \
		case 9: ret = is_same9(p, q); break; \
		case 10: ret = is_same10(p, q); break; \
		case 11: ret = is_same11(p, q); break; \
		case 12: ret = is_same12(p, q); break; \
		case 13: ret = is_same13(p, q); break; \
		case 14: ret = is_same14(p, q); break; \
		case 15: ret = is_same15(p, q); break; \
		case 16: ret = is_same16(p, q); break; \
		case 17: ret = is_same17(p, q); break; \
		case 18: ret = is_same18(p, q); break; \
		case 19: ret = is_same19(p, q); break; \
		case 20: ret = is_same20(p, q); break; \
		case 21: ret = is_same21(p, q); break; \
		case 22: ret = is_same22(p, q); break; \
		case 23: ret = is_same23(p, q); break; \
		case 24: ret = is_same24(p, q); break; \
		case 25: ret = is_same25(p, q); break; \
		case 26: ret = is_same26(p, q); break; \
		case 27: ret = is_same27(p, q); break; \
		case 28: ret = is_same28(p, q); break; \
		case 29: ret = is_same29(p, q); break; \
		case 30: ret = is_same30(p, q); break; \
		case 31: ret = is_same31(p, q); break; \
		case 32: ret = is_same32(p, q); break; \
		default: \
			ret = memcmp(p1, p2, n); \
			break; \
		} \
	} else { \
		ret =  memcmp(p1, p2, n); \
	} \
	ret == 0; \
})

#else

inline bool is_same(const void *p1, const void *p2, size_t n)
{
	const uint8_t *p = (const uint8_t *)p1;
	const uint8_t *q = (const uint8_t *)p2;
	switch (n) { \
	case 1: return is_same1(p, q);
	case 2: return is_same2(p, q);
	case 3: return is_same3(p, q);
	case 4: return is_same4(p, q);
	case 5: return is_same5(p, q);
	case 6: return is_same6(p, q);
	case 7: return is_same7(p, q);
	case 8: return is_same8(p, q);
	case 9: return is_same9(p, q);
	case 10: return is_same10(p, q);
	case 11: return is_same11(p, q);
	case 12: return is_same12(p, q);
	case 13: return is_same13(p, q);
	case 14: return is_same14(p, q);
	case 15: return is_same15(p, q);
	case 16: return is_same16(p, q);
	case 17: return is_same17(p, q);
	case 18: return is_same18(p, q);
	case 19: return is_same19(p, q);
	case 20: return is_same20(p, q);
	case 21: return is_same21(p, q);
	case 22: return is_same22(p, q);
	case 23: return is_same23(p, q);
	case 24: return is_same24(p, q);
	case 25: return is_same25(p, q);
	case 26: return is_same26(p, q);
	case 27: return is_same27(p, q);
	case 28: return is_same28(p, q);
	case 29: return is_same29(p, q);
	case 30: return is_same30(p, q);
	case 31: return is_same31(p, q);
	case 32: return is_same32(p, q);
	}
	return memcmp(p, q, n) == 0;
}
#endif

int main(int , char *argv[])
{
	return is_same(&argv[0][0], &argv[0][1], 17);
}
