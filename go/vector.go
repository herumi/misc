package main

/*
#include <stdio.h>
#include <malloc.h>
struct A {
	int x;
	int y;
};
typedef struct A A;
A *createA(int x)
{
	A *p = calloc(sizeof(A), 1);
	p->x = x;
	p->y = x;
	return p;
}

void destroyA(A *a)
{
	free(a);
}

void putA(const A *a)
{
	printf("addr %p %d\n", a, a->x);
}
void putV(const A *const *a, size_t n)
{
	size_t i = 0;
	for (i = 0; i < n; i++) {
		putA(a[i]);
	}
}
*/
import "C"
import "fmt"
import "runtime"
import "unsafe"

type A struct {
	self *C.A
}

func destroyA(a *A) {
	C.destroyA(a.self)
}
func newA(x int) *A {
	a := new(A)
	a.self = C.createA(C.int(x))
	runtime.SetFinalizer(a, destroyA)
	return a
}

func (a *A) put() {
	C.putA(a.self);
}

func main() {
	n := 10
	fmt.Println("make([]A, n)")
	{
		v := make([]A, n)
		for i := 0; i < len(v); i++ {
			v[i] = *newA(i)
		}
		fmt.Println("&v   =", &v)
		fmt.Println("&v[0]=", &v[0])
		for i := 0; i < len(v); i++ {
			v[i].put()
		}
	}
	fmt.Println("make([]*A, n)")
	{
		v := make([]*A, 10)
		for i := 0; i < len(v); i++ {
			v[i] = newA(i)
		}
		fmt.Println("&v   =", &v)
		fmt.Println("&v[0]=", &v[0])
		for i := 0; i < len(v); i++ {
			v[i].put()
		}
	}
	fmt.Println("make([]*C.A, n)")
	{
		v := make([]*C.A, n)
		for i := 0; i < len(v); i++ {
			v[i] = newA(i).self
		}
		fmt.Println("&v   =", &v)
		fmt.Println("&v[0]=", &v[0])
		C.putV((**C.A)(unsafe.Pointer(&v[0])), C.size_t(n))
	}
}
