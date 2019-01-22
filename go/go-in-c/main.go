package main

/*
// https://github.com/golang/go/wiki/cgo#function-variables
void fill(void *buf, int n);
void ggg(void f(void *, int), void *buf, int n);
void hhh();
int fff();
void putput(const void *buf, int n);
void putBuf();
void callGoFunc3(void (*f)(void *), void *buf, int n);
#cgo LDFLAGS:-L./ -llib
*/
import "C"
import "fmt"
import "unsafe"
import "crypto/rand"

func Fill(buf []byte, n int) {
	for i := 0; i < n; i++ {
		buf[i] = byte(i + 5)
	}
}

type Pfunc func([]byte, int)

type Op struct {
	f func([]byte, int)
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
	size := int(n)
	fmt.Printf("buf=%p\n", buf)
	fmt.Printf("n=%d\n", n)
	slice := (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
	for i := 0; i < size; i++ {
		fmt.Printf("%02x ", slice[i])
	}
	fmt.Printf("\n")
	fmt.Printf("slice=%x\n", slice)
}

//export GoFunc3
func GoFunc3(callback func([]byte), buf *C.char, n C.int) {
	fmt.Printf("call GoFunc3\n")
	size := int(n)
	slice := (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
	fmt.Printf("slice=%x\n", slice)
	callback(slice)
}

func main() {
	fmt.Printf("%d\n", C.fff())
	s := make([]byte, 4)
	fmt.Printf("s=%x\n", s)
	C.fill(unsafe.Pointer(&s[0]), C.int(len(s)))
//	C.ggg(C.fill, unsafe.Pointer(&s[0]), C.int(len(s)))
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
	C.putBuf();
//	C.callGoFunc3(op.f)
	C.putBuf();
}
