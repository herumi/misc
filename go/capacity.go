package main

/*
#include <stdio.h>
static char buf[] = "0123456789ab";

void putBuf()
{
	printf("buf=");
	for (size_t i = 0; i < sizeof(buf); i++) {
		printf("%02x ", (unsigned char)buf[i]);
	}
	printf("\n");
}
const char *getPtr() { return buf; }
size_t getSize() { return sizeof(buf) - 5; }
*/
import "C"
import (
	"fmt"
	"unsafe"
)

func createSliceNoCap(buf *C.char, n C.size_t) []byte {
	size := int(n)
	/*
		non-constant array bound size is not allowed
	*/
	//	return (*[size]byte)(unsafe.Pointer(buf))
	/*
		cast to a large constant-size array
		and get the slice
	*/
	return (*[1 << 30]byte)(unsafe.Pointer(buf))[:size]
}

func createSlice(buf *C.char, n C.size_t) []byte {
	size := int(n)
	return (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
}

func main() {
	C.putBuf()
	ptr := C.getPtr()
	n := C.getSize()
	s := createSliceNoCap(ptr, n)
	fmt.Printf("s=%x %p %d\n", s, s, cap(s))
	s = append(s, '\x99')
	fmt.Printf("s=%x %p\n", s, s)
	C.putBuf()
	s = createSlice(ptr, n)
	fmt.Printf("s=%x %p %d\n", s, s, cap(s))
	s = append(s, '\x88')
	fmt.Printf("s=%x %p %d\n", s, s, cap(s))
	C.putBuf()
}
