package BN254

import (
	"fmt"
	"strconv"
	"testing"
)

func TestMain(t *testing.T) {
	if Init() == BLS_FAIL {
		fmt.Printf("err")
		return
	}
	H := NewHashAndMap()
	for i := 0; i < 10000; i++ {
		s := strconv.Itoa(i)
		P := H.SetHashOf([]byte(s))
		fmt.Printf("%d %v\n", i, P.ToString())
	}
}
