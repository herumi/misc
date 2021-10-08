#include <stdint.h>
#include <math.h>
#include <vector>
#include <stdio.h>
#include <memory.h>
#include <string>

uint32_t F(uint32_t val, uint32_t pos) { return val << pos; }

uint32_t concat(const std::initializer_list<uint32_t> list) {
  uint32_t result = 0;
  for (auto f : list) {
    result |= f;
  }
  return result;
}

inline uint64_t ones(uint32_t size) { return (size == 64) ? 0xffffffffffffffff : ((uint64_t)1 << size) - 1; }

inline uint32_t field(uint64_t v, uint32_t mpos, uint32_t lpos) { return static_cast<uint32_t>((v >> lpos) & ones(mpos - lpos + 1)); }

uint32_t compactImm(double imm, uint32_t size) {
  uint32_t sign = (imm < 0) ? 1 : 0;

  imm = std::abs(imm);
  int32_t max_digit = static_cast<int32_t>(std::floor(std::log2(imm)));

  int32_t n = (size == 16) ? 7 : (size == 32) ? 10 : 13;
  int32_t exp = (max_digit - 1) + (1 << n);

  imm -= pow(2, max_digit);
  uint32_t frac = 0;
  for (int i = 0; i < 4; ++i) {
    if (pow(2, max_digit - 1 - i) <= imm) {
      frac |= 1 << (3 - i);
      imm -= pow(2, max_digit - 1 - i);
    }
  }
  uint32_t imm8 = concat({F(sign, 7), F(field(~exp, n, n), 6), F(field(exp, 1, 0), 4), F(frac, 0)});
  return imm8;
}

#define ERR_ILLEGAL_IMM_VALUE 3

inline uint64_t mask(int x) {
  return (uint64_t(1) << x) - 1;
}
// 8bitFloat = 1(sign) + 3(exponent) + 4(mantissa)
inline uint32_t code8bitFloat(double x) {
  uint64_t u;
  memcpy(&u, &x, sizeof(u));
  uint32_t sign = (u >> 63)&1;
  int e = int((u >> 52) & mask(11)) - 1023;
  if (e < -3 || e > 4) throw(ERR_ILLEGAL_IMM_VALUE);
  e=(e+7)&7;
  uint64_t m = u & mask(52);
  if (m & mask(48)) throw(ERR_ILLEGAL_IMM_VALUE);
  return uint32_t((sign << 7) | (e << 4) | (m >> 48));
}

std::string toBin(uint8_t v)
{
	char buf[8];
	for (int i = 0; i < 8; i++) {
		buf[7 - i] = (v & (1 << i)) ? '1' : '0';
	}
	return std::string(buf, 8);
}

void putPtn(double x)
{
	uint32_t v1 = compactImm(x, 16);
	uint32_t v2 = compactImm(x, 32);
	uint32_t v3 = compactImm(x, 64);
	if (v1 != v2 || v1 != v3) {
		printf("err x=%f v1=%d v2=%d v3=%d\n", x, v1, v2, v3);
		exit(1);
	}
	uint32_t v = code8bitFloat(x);
	if (v != v1) {
		printf("(%c) x=%f v1=%s v=%s\n", v == v1 ?'o' : 'x', x, toBin(v1).c_str(), toBin(v).c_str());
		int a = (v1 >> 4)&7;
		int b = (v >> 4)&7;
		printf("(%c) v1:e=%d v:e=%d\n", a == b ?'o' : 'x', a, b);
		exit(1);
	}
	printf("x=%f v=%x\n", x, v1);
}

void ptn()
{
	for (int n = 16; n <= 31; n++) {
		for (int r = -3; r <= 4; r++) {
			double x = n / 16.0 * pow(2, r);
			putPtn(x);
//			putPtn(-x);
		}
	}
}

int main()
{
	ptn();
}
