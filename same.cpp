#include <stdio.h>
#include <stdint.h>
#include <memory.h>

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
#define is_same(p1, p2, n) \
({ \
	const uint8_t *p = (const uint8_t *)p1; \
	const uint8_t *q = (const uint8_t *)p2; \
	uint64_t ret; \
	if (__builtin_constant_p(n)) { \
		switch (n) { \
		case 1: ret = get1(p) ^ get1(q); break; \
		case 2: ret = get2(p) ^ get2(q); break; \
		case 3: ret = get3(p) ^ get3(q); break; \
		case 4: ret = get4(p) ^ get4(q); break; \
		case 5: ret = get5(p) ^ get5(q); break; \
		case 6: ret = get6(p) ^ get6(q); break; \
		case 7: ret = get7(p) ^ get7(q); break; \
		case 8: ret = get8(p) ^ get8(q); break; \
		case 9: ret = (get8(p) ^ get8(q)) | (get1(p + 8) ^ get1(q + 8)); break; \
		case 10: ret = (get8(p) ^ get8(q)) | (get2(p + 8) ^ get2(q + 8)); break; \
		case 11: ret = (get8(p) ^ get8(q)) | (get3(p + 8) ^ get3(q + 8)); break; \
		case 12: ret = (get8(p) ^ get8(q)) | (get4(p + 8) ^ get4(q + 8)); break; \
		case 13: ret = (get8(p) ^ get8(q)) | (get5(p + 8) ^ get5(q + 8)); break; \
		case 14: ret = (get8(p) ^ get8(q)) | (get6(p + 8) ^ get6(q + 8)); break; \
		case 15: ret = (get8(p) ^ get8(q)) | (get7(p + 8) ^ get7(q + 8)); break; \
		case 16: ret = (get8(p) ^ get8(q)) | (get8(p + 8) ^ get8(q + 8)); break; \
		default: \
			ret = memcmp(p1, p2, n); \
			break; \
		} \
	} else { \
		ret =  memcmp(p1, p2, n); \
	} \
	ret == 0; \
})

int main(int , char *argv[])
{
	return is_same(&argv[0][0], &argv[0][1], 16);
}
