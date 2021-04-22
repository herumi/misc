package BN254

import (
	"testing"
	"fmt"
)

func TestMain(t *testing.T) {
	if Init() == BLS_FAIL {
		fmt.Printf("err")
		return
	}
	x := StringToFP("ac00f2c9af814438db241461ec7825ed88d00b0951049aa1b5116e6dca345ea", 16)

	fmt.Printf("x=%v\n", x.ToString())
	H := NewHashAndMap()
	P := H.MapToG1(x)
	fmt.Printf("P=%v\n", P.ToString())
	for i := 1; i < 10; i++ {
		x = NewFPint(i)
		fmt.Printf("x=%v\n", x.ToString())
		P = H.MapToG1(x)
		fmt.Printf("P=%v\n", P.ToString())
	}
}

