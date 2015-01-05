/*
len=1 max=2(1 * 1 | 1 * 1)
len=2 max=c(1 * 3 |                  1 *                 11)
len=3 max=39(3 * 5 |                 11 *                101)
len=4 max=f8(b * d |                 1011 *              1101)
len=5 max=3f1(17 * 19 |              10111 *             11001)
len=6 max=ff0(37 * 39 |              110111 *            111001)
len=7 max=3fe1(6f * 71 |             1101111 *           1110001)
len=8 max=ffe0(ef * f1 |             11101111 *          11110001)
len=9 max=3ffc1(1df * 1e1 |          111011111 *         111100001)
len=10 max=fffc0(3df * 3e1 |         1111011111 *        1111100001)
len=11 max=3fff90(7a7 * 7e9 |        11110100111 *       11111101001)
len=12 max=ffff80(fbf * fc1 |        111110111111 *      111111000001)
len=13 max=3ffff01(1f7f * 1f81 |     1111101111111 *     1111110000001)
len=14 max=fffff00(3f7f * 3f81 |     11111101111111 *    11111110000001)
len=15 max=3ffffe90(7f27 * 7f69 |    111111100100111 *   111111101101001)
len=16 max=fffffe00(feff * ff01 |    1111111011111111 *  1111111100000001)
len=17 max=3fffffc01(1fdff * 1fe01 | 11111110111111111 * 11111111000000001)

len=64            (31改の1++0++32改の1) * (32改の1++31改の0++1)
*/
#include <stdio.h>
#include <stdint.h>
#include <string>

std::string bin(uint64_t x)
{
	if (x == 0) return "0";
	std::string ret;
	while (x > 0) {
		ret += '0' + (x & 1);
		x >>= 1;
	}
	std::reverse(ret.begin(), ret.end());
	return ret;
}

void f(size_t len)
{
	uint32_t maxi = 0, maxj = 0;
	uint64_t max = 0;
	const uint32_t n = 1 << len;
	const uint64_t mask = n - 1;
	for (uint32_t i = 1; i < n; i++) {
		for (uint32_t j = i; j < n; j++) {
			uint64_t x = uint64_t(i) * j;
			x = (x >> len) | ((x & mask) << len);
			if (x > max) {
				max = x;
				maxi = i;
				maxj = j;
			}
		}
	}
	printf("len=%d max=%llx(%x * %x | %s * %s)\n", (int)len, (long long)max, maxi, maxj, bin(maxi).c_str(), bin(maxj).c_str());
}

int main()
{
	for (int i = 1; i < 20; i++) {
		f(i);
	}
}
