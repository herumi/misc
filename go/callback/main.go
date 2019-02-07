package main
import (
	"fmt"
	"crypto/rand"
)

type RandCallback struct {
}

func (r *RandCallback) read(buf []byte) bool {
	n, err := rand.Read(buf)
	if n != len(buf) || err != nil {
		return false
	}
	return true
}

type Add struct {
	x int
}

func (self *Add) run(x int) int {
	fmt.Printf("      Add.read (self=%x) x=%d\n", self.x, x)
	return x + 1
}

func main() {
	fmt.Printf("callback test\n")
	a := new(Add)
	a.x = 4
	var ifs CallbackIF = a
	setCallbackGo(&ifs)
	/*
		callCallbackGo
		-> C.callCallbackC
		-> s_callbackC ; function pointer of C
		-> C.wrapCallbackCgo ; defined in sub.go
		-> wrapCallbackGo ; defined in callback.go
		-> s_callbackIF
		-> Add.run
	*/
	callCallbackGo(5)
	buf := make([]byte, 7)
	fmt.Printf("buf=%x\n", buf)
	rng := new(RandCallback)
	rng.read(buf)
	fmt.Printf("buf=%x\n", buf)
}

