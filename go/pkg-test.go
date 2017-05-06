package main

// #cgo pkg-config: pkg-test
/*
#include <stdio.h>
typedef struct A {
	int x;
} A;

void add(A* a)
{
	a->x += AAA;
}

void put(const A* a)
{
	printf("%d\n", a->x);
}
*/
import "C"
import "unsafe"
/*
	env PKG_CONFIG_PATH=`pwd` go run pkg-test.go
*/

type A struct {
	x int
}

func (a *A) Add() {
	C.add((*C.A)(unsafe.Pointer(&a.x)))
}
func (a *A) Put() {
	C.put((*C.A)(unsafe.Pointer(&a.x)))
}
func main() {
	var a A
	a.Put()
	a.Add()
	a.Put()
}

