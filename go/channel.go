package main

import "fmt"

func main() {
	n := 5
	v := make([]int, n)
	cs := make(chan int, n)

	add := func(i int) {
		v[i] = v[i] + i
		cs <- 1
	}
	for i := 0; i < n; i++ {
		go add(i)
	}
	for i := 0; i < n; i++ {
		<-cs
	}
	for i := 0; i < n; i++ {
		fmt.Printf("v[%v]=%v\n", i, v[i])
	}
}
