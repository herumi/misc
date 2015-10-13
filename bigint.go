package main

import (
	"fmt"
	"math/big"
	"os"
	"sort"
	"strconv"
)

type Ints []*big.Int

func (v Ints) Len() int {
	return len(v)
}
func (v Ints) Less(i, j int) bool {
	return v[i].Cmp(v[j]) < 0
}
func (v Ints) Swap(i, j int) {
	v[i], v[j] = v[j], v[i]
}

const maxWordNum = 74
type BigIntArray [maxWordNum]big.Word

func toA(x *big.Int) (r BigIntArray) {
	for i, v := range x.Bits() {
		r[i] = v
	}
	return
}

func main() {
	mode := 0
	if len(os.Args) >= 2 {
		mode, _ = strconv.Atoi(os.Args[1])
	}
	const an = 500
	const bn = 500
	switch mode {
	case 0:
		fmt.Println("use map")
		m := map[string]bool{}
		x := big.NewInt(0)
		for a := 2; a <= an; a++ {
			for b := 2; b <= bn; b++ {
				m[x.Exp(big.NewInt(int64(a)), big.NewInt(int64(b)), nil).String()] = true
			}
		}
		fmt.Println(len(m))
	case 1:
		fmt.Println("use sort")
		v := Ints{}
		for a := 2; a <= an; a++ {
			for b := 2; b <= bn; b++ {
				x := new(big.Int).Exp(big.NewInt(int64(a)), big.NewInt(int64(b)), nil)
				v = append(v, x)
			}
		}
		sort.Sort(v)
		n := 1
		for i := 1; i < len(v); i++ {
			if v[i-1].Cmp(v[i]) != 0 {
				n++
			}
		}
		fmt.Println(n)
	case 2:
		fmt.Println("use map with Bytes(), String()")
		m := map[string]bool{}
		x := big.NewInt(0)
		for a := 2; a <= an; a++ {
			for b := 2; b <= bn; b++ {
				m[string(x.Exp(big.NewInt(int64(a)), big.NewInt(int64(b)), nil).Bytes())] = true
			}
		}
		fmt.Println(len(m))
	case 3:
		fmt.Println("use BigIntArray")
		m := map[BigIntArray]bool{}
		x := big.NewInt(0)
		for a := 2; a <= an; a++ {
			for b := 2; b <= bn; b++ {
				m[toA(x.Exp(big.NewInt(int64(a)), big.NewInt(int64(b)), nil))] = true
			}
		}
		fmt.Println(len(m))
	case 4:
		fmt.Println("use Bits().String()")
		m := map[string]bool{}
		x := big.NewInt(0)
		toS := func(x []big.Word) (s string) {
			for _, v := range x {
				s += fmt.Sprintf("%x", v)
			}
			return
		}
		for a := 2; a <= an; a++ {
			for b := 2; b <= bn; b++ {
				m[toS(x.Exp(big.NewInt(int64(a)), big.NewInt(int64(b)), nil).Bits())] = true
			}
		}
		fmt.Println(len(m))
	}
}
