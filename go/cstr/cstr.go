package main
/*
#cgo LDFLAGS:-lcstr -L./ -lstdc++
void cstr();
*/
import "C"
import (
    "fmt"
)

func main() {
	fmt.Printf("call C.cstr\n")
	C.cstr();
	fmt.Printf("end\n")
}
