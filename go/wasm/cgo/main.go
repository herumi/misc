package main

/*
#cgo LDFLAGS:-L./ -llib
#include "lib.h"
*/
import "C"
import "fmt"

func main() {
	a := int(C.add(3, 5))
	fmt.Printf("%d\n", a)
}
