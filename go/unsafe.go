package main

/*
#include <stdio.h>
typedef struct A {
	int x[2];
}A ;
void putA(const A *a)
{
	printf("a=%p\n", a);
	printf("x=%d %d\n", a->x[0], a->x[1]);
}
typedef struct B {
	int y[2];
}B ;
void putB(const B *b)
{
	printf("b=%p\n", b);
	printf("y=%d %d\n", b->y[0], b->y[1]);
}
*/
import "C"
import  "unsafe"

type A struct {
	x [2]int
}

func (a *A) Put() {
	C.putA((*C.A)(unsafe.Pointer(&a.x[0])))
}

type B struct {
	y [2]int
}

func (b *B) Put() {
	C.putB((*C.B)(unsafe.Pointer(&b.y[0])))
}


func main() {
	v := make([]A, 2)
	v[0].Put()
	v[0].x[0] = 2
	v[0].x[1] = 5
	v[0].Put()
	w := *(*[]B)(unsafe.Pointer(&v))
	w[0].Put()
}
