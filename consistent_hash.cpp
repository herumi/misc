#include <stdint.h>
#include <stdlib.h>

size_t ConsistentHash(uint64_t key, size_t n) {
	uint64_t b = 1, j = 0;
	while (j < n) {
		b = j;
		key = key * 2862933555777941757ull + 1;
		j = uint64_t((b + 1) * (double(1u << 31) / double((key >> 33) + 1)));
	}
	return (size_t)b;
}

/*
N = 120
n : 10 -> 15

hash[0] 0 7 13 26 39 52 58 65 71 84 97 110 116
hash[1] 4 12 43 51 55 63 75 83 87 95 103 107 115
hash[2] 16 28 38 42 48 50 67 94 99 102 109 119
hash[3] 23 30 35 45 47 49 54 57 60 79 90 92 101 106
hash[4] 5 8 18 37 40 70 81 86 93 100 108 111 118
hash[5] 11 25 29 41 61 72 74 77 96 98
hash[6] 1 2 19 22 34 56 59 64 66 68 69 73 88 89 105 112 117
hash[7] 9 10 15 31 44 46 82 104 114
hash[8] 3 14 21 27 33 36 76 80 91 113
hash[9] 6 17 20 24 32 53 62 78 85

hash[0] 0 13 26 39 58 71 84 97 116
hash[1] 43 51 55 63 75 83 87
hash[2] 16 38 42 67 94 99 102
hash[3] 23 30 35 45 47 49 79 101 106
hash[4] 8 37 40 70 81 86 93 100 108 111
hash[5] 25 41 72 77 96 98
hash[6] 1 2 19 34 56 59 88 105 112
hash[7] 31 44 82 104
hash[8] 3 27 33 36 76 91 113
hash[9] 6 20 32 53
hash[10] 5 12 17 46 48 50 57 89 115 117
hash[11] 21 54 65 68 78 80 85 92 107 118
hash[12] 4 11 24 61 62 73 103
hash[13] 7 14 22 28 60 64 66 109 110 114
hash[14] 9 10 15 18 29 52 69 74 90 95 119
*/

#include <vector>
#include <map>
#include <set>
typedef std::set<size_t> IntSet;
typedef std::map<size_t, IntSet> Map;


void put(const Map& m)
{
	for (const auto& p : m) {
		printf("hash[%d] ", (int)p.first);
		for (size_t x : p.second) {
			printf("%d ", (int)x);
		}
		printf("\n");
	}
}

Map make(size_t n, size_t N)
{
	Map m;
	for (size_t key = 0; key < N; key++) {
		m[ConsistentHash(key, n)].insert(key);
	}
//	put(m);
	return m;
}

int diff(const Map& m1, const Map& m2)
{
	int moved = 0;
	for (const auto& p : m1) {
//		printf("hash[%d] ", p.first);
		auto i2 = m2.find(p.first)->second;
		int n = 0;
		for (size_t x : p.second) {
			if (i2.find(x) == i2.end()) n++;
		}
//		printf("%d\n", n);
		moved += n;
	}
	printf("%d\n", moved);
	return moved;
}

int main(int argc, char *argv[])
{
	const int N = argc >= 2 ? atoi(argv[1]) : 10000;
	const int n1 = argc >= 3 ? atoi(argv[2]) : 100;
	const int n2 = argc >= 4 ? atoi(argv[3]) : 120;
	auto m1 = make(n1, N);
	puts("--");
	auto m2 = make(n2, N);
	int moved = diff(m1, m2);
	printf("expect %d %d\n", int((1 - n1 / double(n2)) * N), moved);
}
