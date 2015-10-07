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
	primes = MakePrime(MakePrimeTable(10000000))
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

func Sum(v []int) int {
	r := 0
	for _, e := range v {
		r += e
	}
	return r
}

func Prod(v []int) int {
	r := 1
	for _, e := range v {
		r *= e
	}
	return r
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
	max := 0
	c := 13
	for i := 0; i < len(v)-c; i++ {
		sv := v[i : i+c]
		x := Prod(sv)
		if x > max {
			max = x
		}
	}
	fmt.Println(max)
}

func prob9() {
	const n = 1000
	for a := 1; a < n; a++ {
		for b := a + 1; b < n; b++ {
			c := n - a - b
			if c <= b {
				break
			}
			if a*a+b*b == c*c {
				fmt.Println(a, b, c, a*b*c)
				return
			}
		}
	}
}

func prob10() {
	const n = 2000000
	sum := 0
	for _, p := range(primes) {
		if p > n {
			break
		}
		sum += p
	}
	fmt.Println(sum)
}

/*
func prob11() {
	tbl := [][]int {
	{ 8,02,22,97,38,15,00,40,00,75,04,05,07,78,52,12,50,77,91, 8},
    {49,49,99,40,17,81,18,57,60,87,17,40,98,43,69,48,04,56,62,00},
    {81,49,31,73,55,79,14,29,93,71,40,67,53,88,30,03,49,13,36,65},
    {52,70,95,23,04,60,11,42,69,24,68,56,01,32,56,71,37,02,36,91},
    {22,31,16,71,51,67,63,89,41,92,36,54,22,40,40,28,66,33,13,80},
    {24,47,32,60,99,03,45,02,44,75,33,53,78,36,84,20,35,17,12,50},
    {32,98,81,28,64,23,67,10,26,38,40,67,59,54,70,66,18,38,64,70},
    {67,26,20,68,02,62,12,20,95,63,94,39,63, 8,40,91,66,49,94,21},
    {24,55,58,05,66,73,99,26,97,17,78,78,96,83,14,88,34,89,63,72},
    {21,36,23, 9,75,00,76,44,20,45,35,14,00,61,33,97,34,31,33,95},
    {78,17,53,28,22,75,31,67,15,94,03,80,04,62,16,14, 9,53,56,92},
    {16,39,05,42,96,35,31,47,55,58,88,24,00,17,54,24,36,29,85,57},
    {86,56,00,48,35,71,89,07,05,44,44,37,44,60,21,58,51,54,17,58},
    {19,80,81,68,05,94,47,69,28,73,92,13,86,52,17,77,04,89,55,40},
    {04,52, 8,83,97,35,99,16,07,97,57,32,16,26,26,79,33,27,98,66},
    {88,36,68,87,57,62,20,72,03,46,33,67,46,55,12,32,63,93,53,69},
    {04,42,16,73,38,25,39,11,24,94,72,18, 8,46,29,32,40,62,76,36},
    {20,69,36,41,72,30,23,88,34,62,99,69,82,67,59,85,74,04,36,16},
    {20,73,35,29,78,31,90,01,74,31,49,71,48,86,81,16,23,57,05,54},
    {01,70,54,71,83,51,54,69,16,92,33,48,61,43,52,01,89,19,67,48}}
	n := len(s)

    dirTbl := [][][]int{
		{{0,0},{0,1},{0,2},{0,3}},
		{{0,0},{1,1},{2,2},{3,3}},
		{{0,0},{1,0},{2,0},{3,0}},
		{{0,0},{1,-1},{2,-2},{3,-3}},
	}

	pickup := func(x0, y0 int, dir []int, tbl[][]int) int {
		r := 1
		for i := 0; i < 4; i++ {
			x := x0 + dir[i]
			y := y0 + dir[j]
			if !(0 <= x && x < n && 0 <= y && y < n) {
				return 0
			}
			r *= tbl[x][y]
		}
		return r
	}
	max := 0
	for x := 0; x < n; x++ {
		for y := 0; y < n; y++ {
			for _, dir := range(dirTbl); {
				r := pickup(x, y, dir, s)
				if r > max {
					max = r
				}
			}
		}
	}
	fmt.Println(max)
}
*/

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
	case 9:
		prob9()
	case 10:
		prob10()
//	case 11:
//		prob11()
	default:
		fmt.Println("not solved")
	}
}
