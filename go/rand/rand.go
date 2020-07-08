package rand

/*
typedef unsigned int (*ReadRandFunc)(void *, void *, unsigned int);
int wrapReadRandCgo(void *self, void *buf, unsigned int n)
*/
import "C"
import (
	"os"
)

func ReadRand() []byte {
	file, err := os.Open("/dev/urandom")
	if err != nil {
		return nil
	}
	defer file.Close()
	buf := make([]byte, 16)
	n, err := file.Read(buf)
	if err != nil {
		return nil
	}
	return buf[:n]
}

var sRandReader io.Reader

func createSlice(buf *C.char, n C.uint) []byte {
	size := int(n)
	return (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
}

// this function can't be put in callback.go
//export wrapReadRandGo
func wrapReadRandGo(buf *C.char, n C.uint) C.uint {
	slice := createSlice(buf, n)
	ret, err := sRandReader.Read(slice)
	if ret == int(n) && err == nil {
		return n
	}
	return 0
}

// SetRandFunc --
func SetRandFunc(randReader io.Reader) {
	sRandReader = randReader
	if randReader != nil {
		C.blsSetRandFunc(nil, C.ReadRandFunc(unsafe.Pointer(C.wrapReadRandCgo)))
	} else {
		// use default random generator
		C.blsSetRandFunc(nil, C.ReadRandFunc(unsafe.Pointer(nil)))
	}
}
