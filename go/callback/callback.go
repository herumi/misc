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

type CallbackIF interface {
	run(int) int
}

var s_callbackIF *CallbackIF

func setCallbackGo(f *CallbackIF) {
	s_callbackIF = f
	C.setCallbackC(C.FuncType(unsafe.Pointer(C.wrapCallbackCgo)))
}

// this function can't be put in sub.go
//export wrapCallbackGo
func wrapCallbackGo(x int) int {
	fmt.Printf("    wrapCallbackGo x=%d\n", x)
	ret := (*s_callbackIF).run(x + 1)
	fmt.Printf("    wrapCallbackGo ret=%d\n", ret)
	return ret
}

func callCallbackGo(x int) {
	C.callCallbackC(C.int(x))
}
