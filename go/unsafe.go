package main

/*
#include <stdio.h>
#include <stdint.h>
typedef struct A {
	uint64_t x[2];
}A ;
void putA(const A *a)
{
	printf("a=%p\n", a);
	printf("x=%d %d\n", (int)a->x[0], (int)a->x[1]);
}
typedef struct B {
	uint64_t y[2];
}B ;
void putB(const B *b)
{
	printf("b=%p\n", b);
	printf("y=%d %d\n", (int)b->y[0], (int)b->y[1]);
}
void putC(const void *p)
{
	putB((const B*)p);
}
*/
import "C"
import  "unsafe"

type A struct {
	x [2]uint64
}

func (a *A) Put() {
	C.putA((*C.A)(unsafe.Pointer(&a.x[0])))
}

type B struct {
	y [2]uint64
}

func (b *B) Put() {
	C.putB((*C.B)(unsafe.Pointer(&b.y[0])))
}
func (b *B) PutC() {
//	C.putC((*C.uint8_t)(unsafe.Pointer(&b.y[0])))
	C.putC((unsafe.Pointer(&b.y[0])))
}


func main() {
	v := make([]A, 2)
	v[0].Put()
	v[0].x[0] = 2
	v[0].x[1] = 5
	v[0].Put()
	w := *(*[]B)(unsafe.Pointer(&v))
	w[0].Put()
	ww := new(B)
	ww.y[0] = 5
	ww.y[1] = 8
	ww.PutC()
}
