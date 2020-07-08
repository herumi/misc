package rand

import (
	"crypto/rand"
	"fmt"
	"io"
	"testing"
)

var sRandReader io.Reader

func TestRand(t *testing.T) {
	/*
		buf := ReadRand()
		if buf == nil {
			t.Fatalf("ReadRand err")
		}
		fmt.Printf("buf=%v\n", buf)
	*/
	buf := make([]byte, 16)
	sRandReader = rand.Reader

	n, err := sRandReader.Read(buf)
	if err != nil {
		t.Fatalf("rand.Read err")
	}
	fmt.Printf("buf2=%v\n", buf[:n])
}
