package BN254

import (
	"math/big"
	"github.com/0chain/gosdk/miracl/core"
	"fmt"
)

func StringToFP(s string, base int) *FP {
	var t big.Int
	t.SetString(s, base)
	tb := FromBytes(t.Bytes())
	return NewFPbig(tb)
}

func FPtoBigInt(x *FP) *big.Int {
	t := new(big.Int)
	t.SetString(x.ToString(), 16)
	return t
}

func BigInttoFP(x *big.Int) *FP {
	return StringToFP(x.String(), 10)
}

type SquareRoot struct {
	p *big.Int
	g *big.Int
	r int
	q *big.Int
	s *big.Int
	q_add_1_div_2 *big.Int
}

func NewSquareRoot() *SquareRoot {
	sq := new(SquareRoot)
	sq.p = new(big.Int)
	sq.p.SetString("2523648240000001ba344d80000000086121000000000013a700000000000013", 16)
	sq.g = big.NewInt(2)
	sq.r = 1
	sq.q = new(big.Int)
	sq.q.SetString("1291b24120000000dd1a26c0000000043090800000000009d380000000000009", 16)
	sq.s = new(big.Int)
	sq.s.SetString("2523648240000001ba344d80000000086121000000000013a700000000000012", 16)
	sq.q_add_1_div_2 = new(big.Int)
	sq.q_add_1_div_2.SetString("948d920900000006e8d1360000000021848400000000004e9c0000000000005", 16)
	return sq
}

// solve x^2 = a mod p and return x
func (sq *SquareRoot) get(a *big.Int) *big.Int {
	x := big.NewInt(0)
	if a.Sign() == 0 {
		return x
	}
	if big.Jacobi(a, sq.p) < 0 {
		return nil
	}
	// (p + 1) / 4 = (q + 1) / 2
	x.Exp(a, sq.q_add_1_div_2, sq.p)
	return x
}

func (sq *SquareRoot) Get(a *FP) *FP {
	b := FPtoBigInt(a)
	c := sq.get(b)
	if c == nil {
		return nil
	}
	return BigInttoFP(c)
}

type HashAndMap struct {
	a *FP
	b *FP
	one *FP
	c1 *FP
	c2 *FP
	sq *SquareRoot
}

func NewHashAndMap() *HashAndMap {
	H := new(HashAndMap)
	H.a = NewFPint(CURVE_A)
	H.b = NewFPbig(NewBIGints(CURVE_B))
	H.one = NewFPint(1)
	var t big.Int
	t.SetString("252364824000000126cd890000000003cf0f0000000000060c00000000000004", 16)
	tb := FromBytes(t.Bytes())
	H.c1 = NewFPbig(tb)
	t.SetString("25236482400000017080eb4000000006181800000000000cd98000000000000b", 16)
	tb = FromBytes(t.Bytes())
	H.c2 = NewFPbig(tb)
	H.sq = NewSquareRoot()
	return H
}

func (H *HashAndMap) getWeierstrass(x *FP) *FP {
	yy := NewFPcopy(x)
	yy.sqr()
	yy.add(H.a)
	yy.mul(x)
	yy.add(H.b)
	return yy
}

func (H *HashAndMap) MapToG1(t *FP) *ECP {
	negative := t.jacobi() < 0
	P := NewECP()
	w := NewFPcopy(t)
	w.sqr()
	w.add(H.b)
	w.add(H.one)
	w.inverse(nil)
	w.mul(H.c1)
	w.mul(t)
	var x FP
	for i := 0; i < 3; i++ {
		switch i {
		case 0:
			x = *NewFPcopy(t)
			x.mul(w)
			x.neg()
			x.add(H.c2)
			break
		case 1:
			x.neg()
			x.sub(H.one)
			break
		case 2:
			x = *NewFPcopy(w)
			x.sqr()
			x.inverse(nil)
			x.add(H.one)
			break
		}
		x.reduce()
		yy := H.getWeierstrass(&x)
		y := H.sq.Get(yy)
		if y != nil {
			if negative {
				y.neg()
			}
			P.x = &x
			P.y = y
			P.z = NewFPint(1)
			return P
		}
	}
	return nil
}

/*
	use b as little endian
	mask b with (1 << bitLen - 1) - 1 where bitLen = 254
	big.Int.SetBytes accepts big endian
*/
func (H *HashAndMap) copyAndMask(b []byte) *FP {
	n := len(b)
	rb := make([]byte, n)
	for i := 0; i < n; i++ {
		rb[i] = b[n - 1 - i]
	}
	fmt.Printf("rb=%x\n", rb)
	const bitLen = 254
	mask := 1 << ((bitLen % 8) - 1) - 1
	fmt.Printf("mask=%x\n", mask)
	rb[0] = byte(int(rb[0]) & mask)
	fmt.Printf("rb2=%x\n", rb)
	xb := new(big.Int)
	xb.SetBytes(rb)
	if xb.Cmp(H.sq.p) >= 0 {
		xb.Sub(xb, H.sq.p)
	}
	fmt.Printf("x=%v\n", xb.String())
	x := BigInttoFP(xb)
	return x
}

func (H *HashAndMap) SetHashOf(msg []byte) *ECP {
fmt.Printf("msg=%x\n", msg)
	hash := core.NewHASH256()
	hash.Process_array(msg)
	md := hash.Hash()
	fmt.Printf("md=%x\n", md)
	x := H.copyAndMask(md)
	return H.MapToG1(x)
}

