package main

/*
void waitTask(int n);
#cgo LDFLAGS:-lwait -L./ -lstdc++
*/
import "C"

func main() {
	C.waitTask(8)
}
