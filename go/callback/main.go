package main
import (
	"fmt"
)

func testCallbackGo(x int) int {
	fmt.Printf("      testCallbackGo x=%d\n", x)
	return x + 1
}

func main() {
	fmt.Printf("callback test\n")
	setCallbackGo(testCallbackGo)
	/*
		callCallbackGo -> callCallbackC -> s_callbackC -> wrapCallbackCgo -> wrapCallbackGo -> s_callbackGo -> testCallbackGo
	*/
	callCallbackGo(5)
}

