package main

import (
	"fmt"
	"os"
)

func readRand() []byte {
	file, err := os.Open("/dev/urandom")
	if err != nil {
		fmt.Printf("can't open\n")
		return nil
	}
	defer file.Close()
	buf := make([]byte, 16)
	n, err := file.Read(buf)
	if err != nil {
		fmt.Printf("read err\n")
		return nil
	}
	return buf[:n]
}

func main() {
	buf := readRand()
	fmt.Printf("buf=%v\n", buf)
}
