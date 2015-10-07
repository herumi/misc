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
	p := fs[len(fs)-1].p
	fmt.Println(p)
}

func prob4() {
	isPalindrome := func(x int) bool {
		s := strconv.Itoa(x)
		n := len(s)
		for i := 0; i < n/2; i++ {
			if s[i] != s[n-1-i] {
				return false
			}
		}
		return true
	}
	ret := 0
	for a := 100; a < 1000; a++ {
		for b := a; b < 1000; b++ {
			x := a * b
			if isPalindrome(x) {
				if x > ret {
					ret = x
				}
			}
		}
	}
	fmt.Println(ret)
}

func gcd(x, y int) int {
	if x == 0 {
		return y
	}
	return gcd(y%x, x)
}
func lcm(x, y int) int {
	return x * y / gcd(x, y)
}
func prob5() {
	r := 1
	for i := 2; i <= 20; i++ {
		r = lcm(r, i)
	}
	fmt.Println(r)
}

func prob6() {
	sum1 := func(n int) int {
		return n * (n + 1) / 2
	}
	sum2 := func(n int) int {
		return n * (n + 1) * (2*n + 1) / 6
	}
	ans := func(n int) int {
		s1 := sum1(n)
		s2 := sum2(n)
		return s1*s1 - s2
	}
	fmt.Println(ans(100))
}

func prob7() {
	fmt.Println(primes[5], primes[10000])
}

func prob8() {
	s := `
    73167176531330624919225119674426574742355349194934
    96983520312774506326239578318016984801869478851843
    85861560789112949495459501737958331952853208805511
    12540698747158523863050715693290963295227443043557
    66896648950445244523161731856403098711121722383113
    62229893423380308135336276614282806444486645238749
    30358907296290491560440772390713810515859307960866
    70172427121883998797908792274921901699720888093776
    65727333001053367881220235421809751254540594752243
    52584907711670556013604839586446706324415722155397
    53697817977846174064955149290862569321978468622482
    83972241375657056057490261407972968652414535100474
    82166370484403199890008895243450658541227588666881
    16427171479924442928230863465674813919123162824586
    17866458359124566529476545682848912883142607690042
    24219022671055626321111109370544217506941658960408
    07198403850962455444362981230987879927244284909188
    84580156166097919133875499200524063689912560717606
    05886116467109405077541002256983155200055935729725
    71636269561882670428252483600823257530420752963450
   `
	conv := func(s string) (v []int) {
		for _, c := range s {
			if '0' <= c && c <= '9' {
				v = append(v, int(c-'0'))
			}
		}
		return v
	}
	v := conv(s)
	prod := func(v []int) int {
		r := 1
		for _, e := range v {
			r *= e
		}
		return r
	}
	max := 0
	c := 13
	for i := 0; i < len(v)-c; i++ {
		sv := v[i : i+c]
		x := prod(sv)
		if x > max {
			max = x
		}
	}
	fmt.Println(max)
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
	case 4:
		prob4()
	case 5:
		prob5()
	case 6:
		prob6()
	case 7:
		prob7()
	case 8:
		prob8()
	default:
		fmt.Println("not solved")
	}
}
