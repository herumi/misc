package main

/*
#include <stdio.h>
#include <malloc.h>
struct A {
	char buf[8];
};
typedef struct A A;
A *createA()
{
	A *p = calloc(sizeof(A), 1);
	return p;
}

void destroyA(A *a)
{
	free(a);
}

void putA(const A *a)
{
	printf("addr %p\n", a);
}
*/
import "C"
import "fmt"
import "runtime"

type A struct {
	self *C.A
}

func destroyA(a *A) {
	C.destroyA(a.self)
}
func newA() *A {
	a := new(A)
	a.self = C.createA()
	runtime.SetFinalizer(a, destroyA)
	return a
}

func (a *A) put() {
	C.putA(a.self);
}

func main() {
	v := make([]A, 10)
	for i := 0; i < len(v); i++ {
		v[i] = *newA()
	}
	fmt.Println("&v   =", &v)
	fmt.Println("&v[0]=", &v[0])
	for i := 0; i < len(v); i++ {
		v[i].put()
	}
}
