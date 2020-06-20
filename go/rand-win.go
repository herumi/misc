package main

/*
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#include <stdint.h>
#pragma comment (lib, "advapi32.lib")

int readRand(unsigned char *p, size_t n)
{
	HCRYPTPROV prov_ = 0;
	DWORD flagTbl[] = { 0, CRYPT_NEWKEYSET, CRYPT_MACHINE_KEYSET };
	int found = 0;
	for (int i = 0; i < 3; i++) {
		printf("i=%d flag=%d\n", i, flagTbl[i]);
		if (CryptAcquireContext(&prov_, NULL, NULL, PROV_RSA_FULL, flagTbl[i]) != 0) {
			printf("found %d prov_=%p\n", i, prov_);
			found = 1;
			break;
		}
	}
	if (!found) return 0;
	DWORD byteSize = (DWORD)n;
	int ret = CryptGenRandom(prov_, byteSize, p);
	if (ret == 0) {
		printf("CryptGenRandom err %d\n", GetLastError());
		return 0;
	}
	CryptReleaseContext(prov_, 0);
	return 1;
}
*/
import "C"
import (
	"fmt"
	"unsafe"
)

func main() {
	buf := make([]byte, 64)
	ret := C.readRand((*C.uchar)(unsafe.Pointer(&buf[0])), C.size_t(len(buf)))
	fmt.Printf("ret=%v buf=%v\n", ret, buf)
}
