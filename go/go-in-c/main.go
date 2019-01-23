package main

/*
// https://github.com/golang/go/wiki/cgo#function-variables
void fill(void *buf, int n);
void ggg(void f(void *, int), void *buf, int n);
void hhh();
int fff();
void putput(const void *buf, int n);
void putBuf();
void callGoFunc3(void *f);
typedef void (*FuncType)(void *, int);
void callCallback(FuncType f);
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
	return (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
}

//export GoCallF
func GoCallF(buf *C.char, n C.int) {
	g_f(createSlice(buf, n))
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

//export GoFunc3
func GoFunc3(callback *C.char, buf *C.char, n C.int) {
	fmt.Printf("call GoFunc3\n")
	slice := createSlice(buf, n)
	fmt.Printf("slice=%x\n", slice)
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

	SetOp(rand.Read)
	C.putBuf();
	C.callGoCallF()
	C.putBuf();
	SetOp(RandFill)
	C.callGoCallF()
	C.putBuf();
}