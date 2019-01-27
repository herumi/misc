package main

/*
// https://github.com/golang/go/wiki/cgo#function-variables
#include "lib.h"
void fill(void *buf, int n);
void ggg(void f(void *, int), void *buf, int n);
void hhh();
int fff();
void putput(const void *buf, int n);
void putBuf();
void callGoCallF();
#cgo LDFLAGS:-L./ -llib
*/
import "C"
import "fmt"
import "unsafe"
import "crypto/rand"

type FuncType func([]byte) (int, error)

var g_f FuncType

func SetOp(f FuncType) {
	g_f = f
}

func RandFill(buf []byte) (int, error) {
	for i := 0; i < len(buf); i++ {
		buf[i] = byte(i + 9)
	}
	return len(buf), nil
}

func Fill(buf []byte, n int) {
	for i := 0; i < n; i++ {
		buf[i] = byte(i + 5)
	}
}

type Op struct {
	f func([]byte, int)
}

func createSlice(buf *C.char, n C.int) []byte {
	size := int(n)
//	return (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
	return (*[1 << 30]byte)(unsafe.Pointer(buf))[:size]
}

//export GoCallF
func GoCallF(buf *C.char, n C.int) {
	ret, err := g_f(createSlice(buf, n))
	if err != nil {
		panic("err")
	}
	if ret != int(n) {
		panic("err2")
	}
}

/*
	This function is called from fff() in C
*/
//export GoFunc1
func GoFunc1() {
	fmt.Printf("call GoFunc1\n")
}
//export GoFunc2
func GoFunc2(buf *C.char, n C.int) {
	fmt.Printf("call GoFunc2\n")
	fmt.Printf("buf=%p\n", buf)
	fmt.Printf("n=%d\n", n)
	slice := createSlice(buf, n)
	for i := 0; i < len(slice); i++ {
		fmt.Printf("%02x ", slice[i])
	}
	fmt.Printf("\n")
	fmt.Printf("slice=%x\n", slice)
}

func main() {
	fmt.Printf("%d\n", C.fff())
	s := make([]byte, 4)
	fmt.Printf("s=%x\n", s)
	C.fill(unsafe.Pointer(&s[0]), C.int(len(s)))
	C.ggg((C.FuncType)(C.fill), unsafe.Pointer(&s[0]), C.int(len(s)))
	fmt.Printf("s=%x\n", s)
	Fill(s, len(s))
	fmt.Printf("s=%x\n", s)
	rand.Read(s)
	fmt.Printf("s=%x\n", s)
	var op Op
	op.f = Fill
	op.f(s, len(s))
	fmt.Printf("s=%x\n", s)
	C.hhh() // call GoFunc1
	C.putput(unsafe.Pointer(&s[0]), C.int(len(s)))

	SetOp(rand.Read)
	C.putBuf();
	C.callGoCallF()
	C.putBuf();
	SetOp(RandFill)
	C.callGoCallF()
	C.putBuf();

	// pass NULL
	C.putput(nil, C.int(0))
//	C.setCallback((C.FuncType)(unsafe.Pointer(C.wrapCallback)))
}
