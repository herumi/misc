#include <stdio.h>
#include <openssl/sha.h>

int main()
{
	unsigned char md[128];
	const unsigned char *src = (const unsigned char*)"abc";
	SHA256(src, 3, md);
	for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		printf("%02X", (unsigned char)md[i]);
	}
	printf("\n");
}
