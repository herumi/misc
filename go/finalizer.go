package main

/*
#include <stdio.h>
#include <malloc.h>
struct A;
static int c = 0;
struct A *createA()
{
	struct A *p = calloc(10, 1);
	printf("create A %p %d\n", p, c++);
	return p;
}

void destroyA(struct A *a)
{
	printf("destroy A %p %d\n", a, --c);
	free(a);
}
*/
import "C"
import "fmt"
import "runtime"
import "time"

type A struct {
	self *C.struct_A
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

func main() {
	var a *A
	for i := 0; i < 10; i++ {
		a = newA()
	}
	fmt.Println("runtime GC", a)
	runtime.GC()
	fmt.Println("sleep")
	time.Sleep(2 * time.Second)
}
