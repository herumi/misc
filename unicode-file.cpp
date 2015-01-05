#include <stdio.h>
#include <string>
#include <fstream>

int main()
{
	const wchar_t tbl[][10] = {
		{ 0x01d6 },
		{ 0x00fc, 0x0304 },
		{ 0x0075, 0x0308, 0x0304 },
		{ },
	};
	for (const wchar_t *file : tbl) {
		std::wofstream f(file, std::ios::binary);
	}
}
