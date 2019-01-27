package main
/*
#include "lib.h"
#cgo CFLAGS: -I .
#cgo LDFLAGS: -L./ -llib
int wrapCallbackCgo(int in);
*/
import "C"

import (
	"fmt"
	"unsafe"
)

type FuncType func(int) int

var s_callbackGo FuncType

func setCallbackGo(f FuncType) {
	s_callbackGo = f
	C.setCallbackC(C.FuncType(unsafe.Pointer(C.wrapCallbackCgo)))
}

// this function can't be put in sub.go
//export wrapCallbackGo
func wrapCallbackGo(x int) int {
	fmt.Printf("    wrapCallbackGo x=%d\n", x)
	ret := s_callbackGo(x + 1)
	fmt.Printf("    wrapCallbackGo ret=%d\n", ret)
	return ret
}

func callCallbackGo(x int) {
	C.callCallbackC(C.int(x))
}
