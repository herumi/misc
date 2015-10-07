package main

import (
	"fmt"
	"os"
	"strconv"
)

type PrimeTable []bool
type Primes []int
type Factor struct {
	p, e int
}

var primes Primes

func init() {
	primes = MakePrime(MakePrimeTable(1000000))
}

func MakePrimeTable(n int) PrimeTable {
	p := make(PrimeTable, n)
	for i := 2; i < n; i++ {
		p[i] = true
	}
	for i := 2; i < n; i++ {
		if p[i] {
			for j := 2 * i; j < n; j += i {
				p[j] = false
			}
		}
	}
	return p
}

func MakePrime(pTbl PrimeTable) (primes Primes) {
	for i := 0; i < len(pTbl); i++ {
		if pTbl[i] {
			primes = append(primes, i)
		}
	}
	return primes
}

func FactorInt(n int, primes Primes) []Factor {
	fs := []Factor{}
	for _, p := range primes {
		if n < p {
			break
		}
		if n%p != 0 {
			continue
		}
		f := Factor{p, 0}
		for {
			n /= p
			f.e++
			if n%p != 0 {
				break
			}
		}
		fs = append(fs, f)
	}
	return fs
}

func prob1() {
	sum := 0
	for i := 1; i < 1000; i++ {
		if i%3 == 0 || i%5 == 0 {
			sum += i
		}
	}
	fmt.Println(sum)
}

func prob2() {
	sum := 0
	x := 1
	y := 2
	for {
		if x%2 == 0 {
			sum += x
		}
		x, y = y, x+y
		if x >= 4000000 {
			break
		}
	}
	fmt.Println(sum)
}


func prob3() {
	n := 600851475143
	fs := FactorInt(n, primes)
	p := fs[len(fs) - 1].p
	fmt.Println(p)
}

func main() {
	if len(os.Args) == 1 {
		fmt.Println("ans num")
		return
	}
	num, err := strconv.Atoi(os.Args[1])
	if err != nil {
		fmt.Println("err num=", err)
		return
	}
	switch num {
	case 1:
		prob1()
	case 2:
		prob2()
	case 3:
		prob3()
	default:
		fmt.Println("not solved")
	}
}
