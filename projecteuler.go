package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"math"
	"math/big"
	"os"
	"sort"
	"strconv"
	"strings"
	//	"io"
	"encoding/csv"
	"os/signal"
	"runtime/pprof"
	"syscall"
)

type PrimeTable []bool
type Primes []int
type Factor struct {
	p, e int
}

var primeTbl PrimeTable
var primes Primes

func init() {
	primeTbl = MakePrimeTable(10000000)
	primes = MakePrime(primeTbl)
}

func isqrt(x int) int {
	return int(math.Sqrt(float64(x)))
}

func ipow(x, y int) int {
	return int(math.Pow(float64(x), float64(y)))
}

func IsPrime(n int) bool {
	if n < len(primeTbl) {
		return primeTbl[n]
	}
	rootn := isqrt(n)
	for _, p := range primes {
		if p > rootn {
			return true
		}
		if n%p == 0 {
			return false
		}
	}
	return true
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

func FactorInt(n int) []Factor {
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

func FactorNum(n int) int {
	fs := FactorInt(n)
	r := 1
	for _, f := range fs {
		r *= f.e + 1
	}
	return r
}

func pow(a, b int) int {
	r := 1
	for i := 0; i < b; i++ {
		r *= a
	}
	return r
}

func SumDivisors(n int) int {
	fs := FactorInt(n)
	r := 1
	for _, f := range fs {
		r *= (pow(f.p, f.e+1) - 1) / (f.p - 1)
	}
	return r - n
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
	fs := FactorInt(n)
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
	for _, p := range primes {
		if p > n {
			break
		}
		sum += p
	}
	fmt.Println(sum)
}

func prob11() {
	tbl := [][]int{
		{8, 02, 22, 97, 38, 15, 00, 40, 00, 75, 04, 05, 07, 78, 52, 12, 50, 77, 91, 8},
		{49, 49, 99, 40, 17, 81, 18, 57, 60, 87, 17, 40, 98, 43, 69, 48, 04, 56, 62, 00},
		{81, 49, 31, 73, 55, 79, 14, 29, 93, 71, 40, 67, 53, 88, 30, 03, 49, 13, 36, 65},
		{52, 70, 95, 23, 04, 60, 11, 42, 69, 24, 68, 56, 01, 32, 56, 71, 37, 02, 36, 91},
		{22, 31, 16, 71, 51, 67, 63, 89, 41, 92, 36, 54, 22, 40, 40, 28, 66, 33, 13, 80},
		{24, 47, 32, 60, 99, 03, 45, 02, 44, 75, 33, 53, 78, 36, 84, 20, 35, 17, 12, 50},
		{32, 98, 81, 28, 64, 23, 67, 10, 26, 38, 40, 67, 59, 54, 70, 66, 18, 38, 64, 70},
		{67, 26, 20, 68, 02, 62, 12, 20, 95, 63, 94, 39, 63, 8, 40, 91, 66, 49, 94, 21},
		{24, 55, 58, 05, 66, 73, 99, 26, 97, 17, 78, 78, 96, 83, 14, 88, 34, 89, 63, 72},
		{21, 36, 23, 9, 75, 00, 76, 44, 20, 45, 35, 14, 00, 61, 33, 97, 34, 31, 33, 95},
		{78, 17, 53, 28, 22, 75, 31, 67, 15, 94, 03, 80, 04, 62, 16, 14, 9, 53, 56, 92},
		{16, 39, 05, 42, 96, 35, 31, 47, 55, 58, 88, 24, 00, 17, 54, 24, 36, 29, 85, 57},
		{86, 56, 00, 48, 35, 71, 89, 07, 05, 44, 44, 37, 44, 60, 21, 58, 51, 54, 17, 58},
		{19, 80, 81, 68, 05, 94, 47, 69, 28, 73, 92, 13, 86, 52, 17, 77, 04, 89, 55, 40},
		{04, 52, 8, 83, 97, 35, 99, 16, 07, 97, 57, 32, 16, 26, 26, 79, 33, 27, 98, 66},
		{88, 36, 68, 87, 57, 62, 20, 72, 03, 46, 33, 67, 46, 55, 12, 32, 63, 93, 53, 69},
		{04, 42, 16, 73, 38, 25, 39, 11, 24, 94, 72, 18, 8, 46, 29, 32, 40, 62, 76, 36},
		{20, 69, 36, 41, 72, 30, 23, 88, 34, 62, 99, 69, 82, 67, 59, 85, 74, 04, 36, 16},
		{20, 73, 35, 29, 78, 31, 90, 01, 74, 31, 49, 71, 48, 86, 81, 16, 23, 57, 05, 54},
		{01, 70, 54, 71, 83, 51, 54, 69, 16, 92, 33, 48, 61, 43, 52, 01, 89, 19, 67, 48}}
	n := len(tbl)

	dirTbl := [][][]int{
		{{0, 0}, {0, 1}, {0, 2}, {0, 3}},
		{{0, 0}, {1, 1}, {2, 2}, {3, 3}},
		{{0, 0}, {1, 0}, {2, 0}, {3, 0}},
		{{0, 0}, {1, -1}, {2, -2}, {3, -3}},
	}

	pickup := func(x0, y0 int, dir [][]int, tbl [][]int) int {
		r := 1
		for i := 0; i < 4; i++ {
			x := x0 + dir[i][0]
			y := y0 + dir[i][1]
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
			for _, dir := range dirTbl {
				r := pickup(x, y, dir, tbl)
				if r > max {
					max = r
				}
			}
		}
	}
	fmt.Println(max)
}

func prob12() {
	const n = 500
	tri := 0
	for i := 1; ; i++ {
		tri += i
		r := FactorNum(tri)
		if r > n {
			fmt.Println(tri)
			return
		}
	}
}

func prob13() {
	tbl := []string{
		"37107287533902102798797998220837590246510135740250",
		"46376937677490009712648124896970078050417018260538",
		"74324986199524741059474233309513058123726617309629",
		"91942213363574161572522430563301811072406154908250",
		"23067588207539346171171980310421047513778063246676",
		"89261670696623633820136378418383684178734361726757",
		"28112879812849979408065481931592621691275889832738",
		"44274228917432520321923589422876796487670272189318",
		"47451445736001306439091167216856844588711603153276",
		"70386486105843025439939619828917593665686757934951",
		"62176457141856560629502157223196586755079324193331",
		"64906352462741904929101432445813822663347944758178",
		"92575867718337217661963751590579239728245598838407",
		"58203565325359399008402633568948830189458628227828",
		"80181199384826282014278194139940567587151170094390",
		"35398664372827112653829987240784473053190104293586",
		"86515506006295864861532075273371959191420517255829",
		"71693888707715466499115593487603532921714970056938",
		"54370070576826684624621495650076471787294438377604",
		"53282654108756828443191190634694037855217779295145",
		"36123272525000296071075082563815656710885258350721",
		"45876576172410976447339110607218265236877223636045",
		"17423706905851860660448207621209813287860733969412",
		"81142660418086830619328460811191061556940512689692",
		"51934325451728388641918047049293215058642563049483",
		"62467221648435076201727918039944693004732956340691",
		"15732444386908125794514089057706229429197107928209",
		"55037687525678773091862540744969844508330393682126",
		"18336384825330154686196124348767681297534375946515",
		"80386287592878490201521685554828717201219257766954",
		"78182833757993103614740356856449095527097864797581",
		"16726320100436897842553539920931837441497806860984",
		"48403098129077791799088218795327364475675590848030",
		"87086987551392711854517078544161852424320693150332",
		"59959406895756536782107074926966537676326235447210",
		"69793950679652694742597709739166693763042633987085",
		"41052684708299085211399427365734116182760315001271",
		"65378607361501080857009149939512557028198746004375",
		"35829035317434717326932123578154982629742552737307",
		"94953759765105305946966067683156574377167401875275",
		"88902802571733229619176668713819931811048770190271",
		"25267680276078003013678680992525463401061632866526",
		"36270218540497705585629946580636237993140746255962",
		"24074486908231174977792365466257246923322810917141",
		"91430288197103288597806669760892938638285025333403",
		"34413065578016127815921815005561868836468420090470",
		"23053081172816430487623791969842487255036638784583",
		"11487696932154902810424020138335124462181441773470",
		"63783299490636259666498587618221225225512486764533",
		"67720186971698544312419572409913959008952310058822",
		"95548255300263520781532296796249481641953868218774",
		"76085327132285723110424803456124867697064507995236",
		"37774242535411291684276865538926205024910326572967",
		"23701913275725675285653248258265463092207058596522",
		"29798860272258331913126375147341994889534765745501",
		"18495701454879288984856827726077713721403798879715",
		"38298203783031473527721580348144513491373226651381",
		"34829543829199918180278916522431027392251122869539",
		"40957953066405232632538044100059654939159879593635",
		"29746152185502371307642255121183693803580388584903",
		"41698116222072977186158236678424689157993532961922",
		"62467957194401269043877107275048102390895523597457",
		"23189706772547915061505504953922979530901129967519",
		"86188088225875314529584099251203829009407770775672",
		"11306739708304724483816533873502340845647058077308",
		"82959174767140363198008187129011875491310547126581",
		"97623331044818386269515456334926366572897563400500",
		"42846280183517070527831839425882145521227251250327",
		"55121603546981200581762165212827652751691296897789",
		"32238195734329339946437501907836945765883352399886",
		"75506164965184775180738168837861091527357929701337",
		"62177842752192623401942399639168044983993173312731",
		"32924185707147349566916674687634660915035914677504",
		"99518671430235219628894890102423325116913619626622",
		"73267460800591547471830798392868535206946944540724",
		"76841822524674417161514036427982273348055556214818",
		"97142617910342598647204516893989422179826088076852",
		"87783646182799346313767754307809363333018982642090",
		"10848802521674670883215120185883543223812876952786",
		"71329612474782464538636993009049310363619763878039",
		"62184073572399794223406235393808339651327408011116",
		"66627891981488087797941876876144230030984490851411",
		"60661826293682836764744779239180335110989069790714",
		"85786944089552990653640447425576083659976645795096",
		"66024396409905389607120198219976047599490197230297",
		"64913982680032973156037120041377903785566085089252",
		"16730939319872750275468906903707539413042652315011",
		"94809377245048795150954100921645863754710598436791",
		"78639167021187492431995700641917969777599028300699",
		"15368713711936614952811305876380278410754449733078",
		"40789923115535562561142322423255033685442488917353",
		"44889911501440648020369068063960672322193204149535",
		"41503128880339536053299340368006977710650566631954",
		"81234880673210146739058568557934581403627822703280",
		"82616570773948327592232845941706525094512325230608",
		"22918802058777319719839450180888072429661980811197",
		"77158542502016545090413245809786882778948721859617",
		"72107838435069186155435662884062257473692284509516",
		"20849603980134001723930671666823555245252804609722",
		"53503534226472524250874054075591789781264330331690"}

	r := new(big.Int)
	x := new(big.Int)
	for _, s := range tbl {
		x.SetString(s, 10)
		r.Add(r, x)
	}
	fmt.Println(r.String()[0:10])
}

func prob14() {
	CountCollatz := func(n int) (r int) {
		for {
			if n == 1 {
				return r
			}
			switch {
			case n%2 == 0:
				n /= 2
			default:
				n = 3*n + 1
			}
		}
	}
	max := 0
	for i := 0; i < 1000000; i++ {
		c := CountCollatz(i)
		if c > max {
			max = c
		}
	}
	fmt.Println(max)
}

func prob15() {
	x := new(big.Int)
	x.Binomial(40, 20)
	fmt.Println(x)
}

func prob16() {
	x := big.NewInt(2)
	x.Exp(x, big.NewInt(1000), nil)
	s := x.String()
	r := 0
	for _, c := range s {
		r += int(c - '0')
	}
	fmt.Println(r)
}

func prob18() {
	s := [][]int{
		{75}, {95, 64}, {17, 47, 82}, {18, 35, 87, 10}, {20, 04, 82, 47, 65}, {19, 01, 23, 75, 03, 34}, {88, 02, 77, 73, 07, 63, 67}, {99, 65, 04, 28, 06, 16, 70, 92}, {41, 41, 26, 56, 83, 40, 80, 70, 33}, {41, 48, 72, 33, 47, 32, 37, 16, 94, 29}, {53, 71, 44, 65, 25, 43, 91, 52, 97, 51, 14}, {70, 11, 33, 28, 77, 73, 17, 78, 39, 68, 17, 57}, {91, 71, 52, 38, 17, 14, 91, 43, 58, 50, 27, 29, 48}, {63, 66, 04, 68, 89, 53, 67, 30, 73, 16, 69, 87, 40, 31}, {04, 62, 98, 27, 23, 9, 70, 98, 73, 93, 38, 53, 60, 04, 23}}

	RemoveTail := func(s [][]int) [][]int {
		n := len(s)
		if n < 2 {
			return s
		}
		r := s[0 : n-2]
		r0 := s[n-2]
		r1 := s[n-1]
		for i := 0; i < len(r0); i++ {
			a := r1[i]
			b := r1[i+1]
			if a > b {
				r0[i] += a
			} else {
				r0[i] += b
			}
		}
		r = append(r, r0)
		return r
	}
	for len(s) > 1 {
		s = RemoveTail(s)
	}
	fmt.Println(s[0][0])
}

func prob20() {
	x := big.NewInt(0)
	x.MulRange(1, 100)
	s := x.String()
	r := 0
	for _, c := range s {
		r += int(c - '0')
	}
	fmt.Println(r)
}

func prob21() {
	amicable := func(a int) bool {
		b := SumDivisors(a)
		return a != b && SumDivisors(b) == a
	}
	s := 0
	for i := 2; i < 10000; i++ {
		if amicable(i) {
			s += i
		}
	}
	fmt.Println(s)
}

func prob22() {
	var sv []string
	const ptn = 0
	switch ptn {
	case 0:
		{
			buf, _ := ioutil.ReadFile("names.txt")
			isv := strings.Split(string(buf), ",")
			for _, s := range isv {
				s = strings.Trim(s, "\"")
				sv = append(sv, s)
			}
		}
	case 1:
		{
			fp, _ := os.Open("names.txt")
			defer fp.Close()
			r := csv.NewReader(fp)
			sv, _ = r.Read()
		}
	case 2:
		{
			fp, _ := os.Open("names.txt")
			defer fp.Close()
			sc := bufio.NewScanner(fp)
			sep := func(data []byte, atEOF bool) (advance int, token []byte, err error) {
				n := len(data)
				s := 0
				if n > 0 && data[0] == ',' {
					s++
				}
				if n > 2 && data[s] == '"' {
					s++
					w := 0
					for data[s+w] != '"' {
						w++
						if s+w == n {
							break
						}
					}
					if s+w < n {
						return s + w + 1, data[s : s+w], nil
					}
				}
				return 0, nil, nil
			}
			sc.Split(sep)
			for sc.Scan() {
				sv = append(sv, sc.Text())
			}
		}
	}
	sort.StringSlice(sv).Sort()
	score := 0
	calc := func(s string) int {
		r := 0
		for _, c := range s {
			r += int(c - 'A' + 1)
		}
		return r
	}
	for i, s := range sv {
		score += (i + 1) * calc(s)
	}
	fmt.Println(score)
}

func prob23() {
	var abundant []int
	const n = 28124
	for i := 1; i < n; i++ {
		if SumDivisors(i) > i {
			abundant = append(abundant, i)
		}
	}
	ptn := make([]bool, n)
	for i := 0; i < len(abundant); i++ {
		a := abundant[i]
		for j := i; j < len(abundant); j++ {
			b := abundant[j]
			if a+b >= n {
				break
			}
			ptn[a+b] = true
		}
	}
	sum := 0
	for i := 0; i < len(ptn); i++ {
		if !ptn[i] {
			sum += i
		}
	}
	fmt.Println(sum)
}

func NextPermutation(v []int) bool {
	n := len(v)
	i := n - 2
	for ; i >= 0; i-- {
		if v[i] < v[i+1] {
			break
		}
	}
	if i < 0 {
		return false
	}
	a := v[i]
	for j := n - 1; j > 0; j-- {
		b := v[j]
		if b > a {
			v[i], v[j] = v[j], v[i]
			for a, b := i+1, n-1; a < b; {
				v[a], v[b] = v[b], v[a]
				a++
				b--
			}
			return true
		}
	}
	return false
}

func prob24() {
	v := make([]int, 10)
	for i := 0; i < 10; i++ {
		v[i] = i
	}
	i := 1
	for {
		NextPermutation(v)
		i++
		if i == 1000000 {
			for _, x := range v {
				fmt.Printf("%d", x)
			}
			fmt.Println()
			break
		}
	}
}

func prob25() {
	x := big.NewInt(1)
	y := big.NewInt(1)
	t := new(big.Int)
	limit := new(big.Int)
	limit.Exp(big.NewInt(10), big.NewInt(1000-1), nil)
	c := 2
	for {
		if x.Cmp(limit) > 0 {
			break
		}
		t.Set(x)
		x.Add(x, y)
		y.Set(t)
		c++
	}
	fmt.Println(c)
}

func prob26() {
	const limit = 1000
	f := func(n int) int {
		m := map[int]bool{}
		r := limit % n
		for {
			_, ok := m[r]
			if ok {
				return len(m)
			}
			m[r] = true
			r = (r * 10) % n
		}
	}
	max := 0
	for i := 2; i < limit; i++ {
		d := f(i)
		if d > max {
			max = i
		}
	}
	fmt.Println(max)
}

func prob27() {
	f := func(a, b int) int {
		abs := func(x int) int {
			if x < 0 {
				return -x
			}
			return x
		}
		for n := 0; n < abs(b); n++ {
			x := n*(n+a) + b
			if !primeTbl[abs(x)] {
				return n
			}
		}
		return 0
	}
	const n = 1000
	max := 0
	ma := 0
	mb := 0
	for a := -n + 1; a < n; a++ {
		for b := -n + 1; b < n; b++ {
			r := f(a, b)
			if r > max {
				max = r
				ma = a
				mb = b
			}
		}
	}
	fmt.Println(ma * mb)
}

func prob28() {
	ans := func(c int) int {
		ac := 2
		ai := 0
		a := func() int {
			ai++
			if ai == 4 {
				ai = 0
				ac += 2
				return ac - 2
			}
			return ac
		}
		b := 1
		s := 0
		for i := 0; i < c*2-1; i++ {
			s += b
			b = b + a()
		}
		return s
	}
	fmt.Println(ans(1001))
}

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

func prob29() {
	if false {
		m := map[string]bool{}
		x := big.NewInt(0)
		for a := 2; a <= 100; a++ {
			for b := 2; b <= 100; b++ {
				m[x.Exp(big.NewInt(int64(a)), big.NewInt(int64(b)), nil).String()] = true
			}
		}
		fmt.Println(len(m))
	} else {
		v := Ints{}
		for a := 2; a <= 100; a++ {
			for b := 2; b <= 100; b++ {
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
	}
}

func prob30() {
	sum := 0
	for i := 2; i <= 9*9*9*9*9*5; i++ {
		s := strconv.Itoa(i)
		t := 0
		for _, e := range s {
			x := int(e - '0')
			xx := x * x
			t += xx * xx * x
		}
		if i == t {
			sum += i
		}
	}
	fmt.Println(sum)
}

func prob31() {
	m := map[int]int{}
	m[0] = 0
	coins := []int{200, 100, 50, 20, 10, 5, 2, 1}
	const Max = 200
	clone := func(m map[int]int) map[int]int {
		r := map[int]int{}
		for v, n := range m {
			r[v] = n
		}
		return r
	}
	for _, c := range coins {
		if c > Max {
			continue
		}
		m2 := clone(m)
		for x := (Max / c) * c; x > 0; x -= c {
			m2[x]++
			for v, n := range m {
				y := x + v
				if y > Max {
					continue
				}
				m2[y] += n
			}
		}
		m = clone(m2)
	}
	fmt.Println(m[200])
}

func prob32() {
	pandigital := func(a, b int) bool {
		s := fmt.Sprintf("%d%d%d", a, b, a*b)
		v := 0
		if len(s) != 9 {
			return false
		}
		for _, e := range s {
			v |= 1 << uint(e-'1')
		}
		return v == (1<<9)-1
	}
	count := func(m map[int]bool, b1, e1, b2, e2 int) {
		for i := b1; i <= e1; i++ {
			for j := b2; j <= e2; j++ {
				if pandigital(i, j) {
					m[i*j] = true
				}
			}
		}
	}
	m := map[int]bool{}
	count(m, 1, 9, 1000, 9999)
	count(m, 10, 99, 100, 999)
	sum := 0
	for v, b := range m {
		if b {
			sum += v
		}
	}
	fmt.Println(sum)
}

func prob33() {
	numer := 1
	denom := 1
	for a := 1; a < 10; a++ {
		for b := 1; b < 10; b++ {
			for c := a + 1; c < 10; c++ {
				/*
					(10a+b)/(10b+c) == a/c
				*/
				if (10*a+b)*c == a*(10*b+c) {
					fmt.Printf("%d%d/%d%d\n", a, b, b, c)
					numer *= a
					denom *= c
				}
			}
		}
	}
	g := gcd(numer, denom)
	fmt.Println(denom / g)
}

func prob34() {
	tbl := [10]int{1}
	for i := 1; i < 10; i++ {
		tbl[i] = tbl[i-1] * i
	}
	valid := func(x int) bool {
		s := fmt.Sprintf("%d", x)
		r := 0
		for _, c := range s {
			r += tbl[int(c-'0')]
		}
		return r == x
	}
	sum := 0
	for i := 3; i < 1000000; i++ {
		if valid(i) {
			sum += i
		}
	}
	fmt.Println(sum)
}

func toInt(x []int) (r int) {
	for i := 0; i < len(x); i++ {
		r = r*10 + x[i]
	}
	return
}

func prob35() {
	rot := func(x []int) {
		t := x[0]
		for i := 1; i < len(x); i++ {
			x[i-1] = x[i]
		}
		x[len(x)-1] = t
	}
	cyclic := func(x int) bool {
		ss := fmt.Sprintf("%d", x)
		n := len(ss)
		s := make([]int, n)
		for i, c := range ss {
			s[i] = int(c - '0')
		}
		for i := 1; i < n; i++ {
			rot(s)
			v := toInt(s)
			if !primeTbl[v] {
				return false
			}
		}
		return true
	}
	n := 0
	for _, p := range primes {
		if p >= 1000000 {
			break
		}
		if cyclic(p) {
			n++
		}
	}
	fmt.Println(n)
}

func prob36() {
	isRev := func(s string) bool {
		n := len(s)
		for i := 0; i < n/2; i++ {
			if s[i] != s[n-1-i] {
				return false
			}
		}
		return true
	}
	valid := func(n int) bool {
		return isRev(fmt.Sprintf("%d", n)) && isRev(fmt.Sprintf("%b", n))
	}
	sum := 0
	for i := 1; i < 1000000; i++ {
		if valid(i) {
			sum += i
		}
	}
	fmt.Println(sum)
}

func prob37() {
	valid1 := func(x int) bool {
		for {
			if !primeTbl[x] {
				return false
			}
			x /= 10
			if x == 0 {
				return true
			}
		}
	}
	valid2 := func(x int) bool {
		for d := 1000000000; d > 0; d /= 10 {
			if d > x {
				continue
			}
			x -= (x / d) * d
			if x == 0 {
				return true
			}
			if !primeTbl[x] {
				return false
			}
		}
		return true
	}
	valid := func(x int) bool {
		return valid1(x) && valid2(x)
	}
	sum := 0
	for _, p := range primes {
		if p < 10 {
			continue
		}
		if valid(p) {
			sum += p
		}
	}
	fmt.Println(sum)
	/*
		{
			tbl := []int{1, 3, 7, 9}
			xs := []int{3, 7}
			add := func(xs []int) []int {
				r := []int{}
				for _, x := range xs {
					for _, s := range tbl {
						y := x * 10 + s
						if primeTbl[y] {
							r = append(r, y)
							if valid(y) {
								fmt.Println("ans", y)
							}
						}
					}
				}
				return r
			}
			for {
				xs = add(xs)
				fmt.Println(xs)
			}

		}
	*/
}

func prob38() {
	max := 0
	for i := 1; i <= 99999; i++ {
		s := ""
		n := 1
		for ; n < 10 && len(s) < 9; n++ {
			s += fmt.Sprintf("%d", i*n)
		}
		if len(s) == 9 {
			v := 0
			for j := 0; j < 9; j++ {
				v |= 1 << uint(s[j]-'0')
			}
			if v == 0x3fe {
				//				fmt.Println(i, n, s)
				si, _ := strconv.Atoi(s)
				if si > max {
					max = si
				}
			}
		}
	}
	fmt.Println(max)
}

func prob39() {
	maxn := 0
	maxp := 0
	for p := 12; p <= 1000; p++ {
		n := 0
		for a := 1; a <= p/3; a++ {
			for b := a + 1; ; b++ {
				c := p - a - b
				if c <= b {
					break
				}
				if a*a == c*c-b*b {
					n++
				}
			}
		}
		if n > maxn {
			maxn = n
			maxp = p
		}
	}
	fmt.Println(maxp)
}

func prob40() {
	gen := func() func() int {
		n := 1
		s := ""
		pos := 0
		d := func() int {
			if len(s) == pos {
				s = fmt.Sprintf("%d", n)
				pos = 0
				n++
			}
			r := int(s[pos] - '0')
			pos++
			return r
		}
		return d
	}
	d := gen()
	ans := 1
	for i := 0; i < 1000000; i++ {
		x := d()
		switch i {
		case 0, 9, 99, 999, 9999, 99999, 999999:
			ans *= x
		}
	}
	fmt.Println(ans)
}

func prob41() {
	f, _ := os.Create("a.prof")
	pprof.StartCPUProfile(f)
	defer pprof.StopCPUProfile()
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		s := <-c
		switch s {
		case syscall.SIGINT:
			pprof.StopCPUProfile()
			os.Exit(1)
		}
	}()

	mk := func(n int) []int {
		v := make([]int, n)
		for i := 0; i < n; i++ {
			v[i] = -(n - i)
		}
		return v
	}
	toI := func(v []int) int {
		r := 0
		for _, x := range v {
			r = r*10 + x
		}
		return -r
	}
	// 1+...+9 = 45 = 0 mod 3
	// 1+...+8 = 36 = 0 mod 3
	// so, you can start with n := 7
	n := 9
	v := mk(n)
	for {
		p := toI(v)
		if v[len(v)-1]%2 != 0 && IsPrime(p) {
			fmt.Println(p)
			break
		}
		b := NextPermutation(v)
		if !b {
			n--
			if n == 1 {
				break
			}
			v = mk(n)
		}
	}
}

func prob42() {
	fp, _ := os.Open("words.txt")
	defer fp.Close()
	r := csv.NewReader(fp)
	sv, _ := r.Read()
	sort.StringSlice(sv).Sort()
	toi := func(s string) (r int) {
		for i := 0; i < len(s); i++ {
			r += int(s[i] - 'A' + 1)
		}
		return
	}
	max := 0
	for _, s := range sv {
		x := toi(s)
		if x > max {
			max = x
		}
	}
	tbl := make([]bool, max+1)
	for i := 0; ; i++ {
		n := i * (i + 1) / 2
		if n > len(tbl) {
			break
		}
		tbl[n] = true
	}
	n := 0
	for _, s := range sv {
		x := toi(s)
		if tbl[x] {
			n++
		}
	}
	fmt.Println(n)
}

func prob43() {
	d := []int{1, 0, 2, 3, 4, 5, 6, 7, 8, 9}

	toi := func(x, y, z int) int {
		return d[x]*100 + d[y]*10 + d[z]
	}
	toi2 := func() int {
		r := d[0]
		for i := 1; i < len(d); i++ {
			r = r*10 + d[i]
		}
		return r
	}
	sum := 0
	for {
		if d[3]%2 == 0 &&
			(d[2]+d[3]+d[4])%3 == 0 &&
			(d[5] == 0 || d[5] == 5) &&
			toi(4, 5, 6)%7 == 0 &&
			toi(5, 6, 7)%11 == 0 &&
			toi(6, 7, 8)%13 == 0 &&
			toi(7, 8, 9)%17 == 0 {
			sum += toi2()
		}
		next := NextPermutation(d)
		if !next {
			break
		}
	}
	fmt.Println(sum)
}

func prob44() {
	pen := func(x int) int {
		return x * (3*x - 1)
	}
	candi := func(x int) int {
		return (1 + isqrt(1+12*x)) / 6
	}
	hasSol := func(x int) bool {
		n := candi(x)
		return pen(n) == x
	}
	for a := 1; ; a++ {
		pa := pen(a)
		for b := a - 1; b > 0; b-- {
			pb := pen(b)
			if hasSol(pa+pb) && hasSol(pa-pb) {
				ans := (pa - pb) / 2
				fmt.Println(ans)
				os.Exit(0)
			}
		}
	}
}

func prob45() {
	invTri := func(a int) int {
		return (isqrt(1+8*a) - 1) / 2
	}
	isTri := func(n, a int) bool {
		return n*n+n == 2*a
	}
	invPen := func(a int) int {
		return (isqrt(1+24*a) + 1) / 6
	}
	isPen := func(n, a int) bool {
		return 3*n*n-n == 2*a
	}
	for i := 1; ; i++ {
		n := i * (2*i - 1)
		if n > 40755 && isTri(invTri(n), n) && isPen(invPen(n), n) {
			fmt.Println(n)
			os.Exit(0)
		}
	}
}

func prob46() {
	for q := 3; ; q += 2 {
		if primeTbl[q] {
			continue
		}
		n := isqrt(q / 2)
		found := false
		for j := 1; j <= n; j++ {
			p := q - 2*j*j
			if primeTbl[p] {
				found = true
				break
			}
		}
		if !found {
			fmt.Println(q)
			break
		}
	}
}

func prob47() {
	n := 2
	for {
		found := true
		for j := 0; j < 4; j++ {
			v := FactorInt(n + j)
			if len(v) != 4 {
				n = n + j + 1
				found = false
				break
			}
		}
		if found {
			break
		}
		n++
	}
	fmt.Println(n)
}

func prob48() {
	r := big.NewInt(0)
	m := big.NewInt(10000000000)
	for i := 1; i <= 1000; i++ {
		x := big.NewInt(int64(i))
		x = x.Exp(x, x, m)
		r = r.Add(r, x)
	}
	s := r.String()
	fmt.Println(s[len(s)-10:])
}

func prob49() {
	mk := func(a, b, c, d int) int {
		return a*1000 + b*100 + c*10 + d
	}
	toa := func(x int) []int {
		r := []int{}
		s := strconv.Itoa(x)
		if len(s) != 4 {
			fmt.Println("x=", x)
			os.Exit(1)
		}
		for i := 0; i < 4; i++ {
			r = append(r, int(s[i]-'0'))
		}
		return r
	}
	eq := func(a, b []int) bool {
		an := len(a)
		bn := len(b)
		if an != bn {
			return false
		}
		for i := 0; i < an; i++ {
			if a[i] != b[i] {
				return false
			}
		}
		return true
	}
	for a := 1; a < 10; a++ {
		for b := 1; b < 10; b++ {
			for c := 1; c < 10; c++ {
				for d := 1; d < 10; d++ {
					x := mk(a, b, c, d)
					if primeTbl[x] {
						v := []int{a, b, c, d}
						for {
							next := NextPermutation(v)
							y := mk(v[0], v[1], v[2], v[3])
							if y > x && primeTbl[y] {
								z := y + (y - x)
								if z < 10000 && primeTbl[z] {
									v1 := toa(z)
									sort.IntSlice(v1).Sort()
									v2 := []int{a, b, c, d}
									sort.IntSlice(v2).Sort()
									if eq(v1, v2) {
										fmt.Printf("%d%d%d\n", x, y, z)
									}
								}
							}
							if !next {
								break
							}
						}
					}
				}
			}
		}
	}
}

func prob50() {
	maxLen := 0
	maxP := 0
	const limit = 1000000
	for i := 0; i < len(primes); i++ {
		s := primes[i]
		q := 0
		n := 0
		for j := i + 1; j < len(primes); j++ {
			s += primes[j]
			if s > limit {
				break
			}
			if primeTbl[s] {
				q = s
				n = j - i + 1
			}
		}
		if n > maxLen {
			maxLen = n
			maxP = q
		}
	}
	fmt.Println(maxP)
}

func prob52() {
	tov := func(x int) (r int) {
		s := strconv.Itoa(x)
		for i := 0; i < len(s); i++ {
			r |= 1 << uint(s[i]-'0')
		}
		return
	}
	x := 1
	for ; ; x++ {
		v := tov(x)
		found := true
		for d := 2; d <= 6; d++ {
			w := tov(x * d)
			if (v | w) != v {
				found = false
				break
			}
		}
		if found {
			break
		}
	}
	fmt.Println(x)
}

func prob53() {
	combi := func(n, r int) *big.Int {
		r1 := new(big.Int)
		r1.MulRange(int64(n-r+1), int64(n))
		r2 := new(big.Int)
		r2.MulRange(1, int64(r))
		r1.Div(r1, r2)
		return r1
	}
	limit := big.NewInt(1000000)
	a := 0
	for n := 1; n <= 100; n++ {
		for r := 1; r <= n; r++ {
			c := combi(n, r)
			if c.Cmp(limit) > 0 {
				a++
			}
		}
	}
	fmt.Println(a)
}

func prob55() {
	rev := func(s string) (r string) {
		n := len(s)
		for i := 0; i < n; i++ {
			r = r + string(s[n-i-1])
		}
		return
	}
	lychrel := func(n int) bool {
		x := big.NewInt(int64(n))
		for c := 0; c <= 50; c++ {
			s := x.String()
			t := rev(s)
			if c > 0 && s == t {
				return false
			}
			ti, _ := new(big.Int).SetString(t, 10)
			x.Add(x, ti)
		}
		return true
	}
	c := 0
	for i := 1; i < 10000; i++ {
		if lychrel(i) {
			c++
		}
	}
	fmt.Println(c)
}

func prob56() {
	sum := func(s string) (r int) {
		for i := 0; i < len(s); i++ {
			r += int(s[i] - '0')
		}
		return
	}
	f := func(a, b int) int {
		x := big.NewInt(int64(a))
		x = x.Exp(x, big.NewInt(int64(b)), nil)
		return sum(x.String())
	}
	max := 0
	for a := 1; a < 100; a++ {
		for b := 1; b < 100; b++ {
			n := f(a, b)
			if n > max {
				max = n
			}
		}
	}
	fmt.Println(max)
}

func prob57() {
	a := big.NewInt(1)
	b := big.NewInt(2)
	t := new(big.Int)
	c := 0
	for i := 0; i < 1000; i++ {
		// sqrt(2) = (b-a)/a
		// a, b = b, 2 * b + a
		t.Set(b)
		b.Add(b, b)
		b.Add(b, a)
		a.Set(t)
		t.Sub(b, a)
		if len(t.String()) > len(a.String()) {
			c++
		}
	}
	fmt.Println(c)
}

func prob58() {
	pn := 0
	total := 1
	n := 1
	for {
		p := 4*n*n - 2*n + 1
		for i := 0; i < 3; i++ {
			if IsPrime(p) {
				pn++
			}
			p += 2 * n
		}
		total += 4
		n++
		if pn*10 < total {
			break
		}
	}
	fmt.Println(2*n - 1)
}

func prob60() {
	min := 100000000
	cat := func(a, b int) int {
		for n := 10; ; n *= 10 {
			if n > a {
				return b*n + a
			}
		}
	}
	L := len(primes)
	f := func(a, b int) bool {
		return IsPrime(cat(a, b)) && IsPrime(cat(b, a))
	}
	for a := 1; a < L; a++ {
		p0 := primes[a]
		if p0*5 > min {
			break
		}
		for b := a + 1; b < L; b++ {
			p1 := primes[b]
			if p1*4 > min-p0 {
				break
			}
			p1mod := p1 % 3
			if !f(p0, p1) {
				continue
			}
			for c := b + 1; c < L; c++ {
				p2 := primes[c]
				if p2*3 > min-p0-p1 {
					break
				}
				p2mod := p2 % 3
				if p1mod != p2mod {
					continue
				}
				if !(f(p0, p2) && f(p1, p2)) {
					continue
				}
				for d := c + 1; d < L; d++ {
					p3 := primes[d]
					if p3*2 > min-p0-p1-p2 {
						break
					}
					p3mod := p3 % 3
					if p3mod != p2mod {
						continue
					}
					if !(f(p0, p3) && f(p1, p3) && f(p2, p3)) {
						continue
					}
					for e := d + 1; e < L; e++ {
						p4 := primes[e]
						if p4 > min-p0-p1-p2-p3 {
							break
						}
						p4mod := p4 % 3
						if p4mod != p3mod {
							continue
						}
						if !(f(p0, p4) && f(p1, p4) && f(p2, p4) && f(p3, p4)) {
							continue
						}
						n := p0 + p1 + p2 + p3 + p4
						fmt.Println(n, p0, p1, p2, p3, p4)
						if n < min {
							min = n
							break
						}
					}
				}
			}
		}
	}
	fmt.Println(min)
}

func prob62() {
	m := map[string][]int{}
	sortStr := func(s string) string {
		n := len(s)
		v := make([]int, n)
		for i := 0; i < n; i++ {
			v[i] = int(s[i])
		}
		sort.Sort(sort.IntSlice(v))
		r := ""
		for i := 0; i < n; i++ {
			r += string(v[i])
		}
		return r
	}
	for n := 1; ; n++ {
		for x := ipow(10, n); x < ipow(10, n+1); x++ {
			s := sortStr(strconv.Itoa(x * x * x))
			m[s] = append(m[s], x)
		}
		min := ipow(10, n+1)
		minv := []int{}
		for _, v := range m {
			if len(v) == 5 && v[0] < min {
				min = v[0]
				minv = v
			}
		}
		if len(minv) > 0 {
			fmt.Println(min * min * min)
			break
		}
	}
}

func prob63() {
	r := map[string]bool{}
	c := int(math.Log(10.0) / (math.Log(10.0) - math.Log(9.0)))
	bigPow := func(x, y int) *big.Int {
		r := new(big.Int)
		r.Exp(big.NewInt(int64(x)), big.NewInt(int64(y)), nil)
		return r
	}
	for m := 1; m <= c; m++ {
		a := bigPow(10, m-1)
		b := new(big.Int).Mul(a, big.NewInt(10))
		for x := 1; x < 10; x++ {
			n := bigPow(x, m)
			if a.Cmp(n) <= 0 && n.Cmp(b) < 0 {
				r[n.String()] = true
			}
		}
	}
	fmt.Println(len(r))
	/*
		// overflow
		r := map[int]bool{}
		c := int(math.Log(10.0) / (math.Log(10.0) - math.Log(9.0)))
		ipow := func(x, y int) int {
			return int(math.Pow(float64(x), float64(y)))
		}
		for m := 1; m <= c; m++ {
			a := ipow(10, m - 1)
			b := a * 10
			for x := 1; x < 10; x++ {
				n := ipow(x, m)
				if a <= n && n < b {
					r[n] = true
				}
			}
		}
		fmt.Println(r)
	*/
}

type P struct {
	a, b, n int
}
type R struct {
	p  P
	ls []P
}

func f(p P) P {
	ap := (p.n - p.b*p.b) / p.a
	an := int((math.Sqrt(float64(p.n)) + float64(p.b)) / float64(ap))
	bp := an*ap - p.b
	return P{ap, bp, an}
}
func g(p P, ls []P) R {
	pp := f(p)
	pn := P{p.a, p.b, pp.n}
	for i := 0; i < len(ls); i++ {
		if pn == ls[i] {
			return R{p, ls}
		}
	}
	return g(P{pp.a, pp.b, p.n}, append(ls, pn))
}
func prob64() {
	periodic := func(n int) []int {
		sn := isqrt(n)
		if sn*sn == n {
			return []int{}
		}
		v := []int{}
		r := g(P{1, isqrt(n), n}, []P{})
		for i := 0; i < len(r.ls); i++ {
			v = append(v, r.ls[i].n)
		}
		return v
	}
	c := 0
	for i := 2; i < 10000; i++ {
		if len(periodic(i))%2 == 1 {
			c++
		}
	}
	fmt.Println(c)
}

type Frac struct {
	nume, deno *big.Int
}

func (p *Frac) Set(nume, deno int) {
	p.nume = big.NewInt(int64(nume))
	p.deno = big.NewInt(int64(deno))
	g := new(big.Int).GCD(nil, nil, p.nume, p.deno)
	p.nume.Div(p.nume, g)
	p.deno.Div(p.deno, g)
}

func (p *Frac) Add(rhs Frac) {
	a := p.nume
	b := p.deno
	c := rhs.nume
	d := rhs.deno
	// a/b + c/d = ad + bc / bd
	a.Mul(a, d)
	d.Mul(d, b)
	b.Mul(b, c)
	a.Add(a, b)
	p.nume.Set(a)
	p.deno.Set(d)
}

func (p *Frac) AddInt(x int) {
	t := big.NewInt(int64(x))
	t.Mul(p.deno, t)
	p.nume.Add(p.nume, t)
}

func (p *Frac) Rev() {
	p.nume, p.deno = p.deno, p.nume
}

func prob65() {
	genRevE := func(n int) func() int {
		i := n - 1
		f := func() int {
			dec := func() { i-- }
			defer dec()
			if i < 0 {
				return 0
			}
			if (i % 3) == 1 {
				return (i/3)*2 + 2
			} else {
				return 1
			}
		}
		return f
	}
	frac := func(n int) *Frac {
		f := genRevE(n - 1)
		a := f()
		r := new(Frac)
		r.Set(1, a)
		for {
			x := f()
			if x == 0 {
				break
			}
			r.AddInt(x)
			r.Rev()
		}
		r.AddInt(2)
		return r
	}
	ans := 0
	s := frac(100).nume.String()
	for i := 0; i < len(s); i++ {
		ans += int(s[i] - '0')
	}
	fmt.Println(ans)
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
	case 9:
		prob9()
	case 10:
		prob10()
	case 11:
		prob11()
	case 12:
		prob12()
	case 13:
		prob13()
	case 14:
		prob14()
	case 15:
		prob15()
	case 16:
		prob16()
	case 18:
		prob18()
	case 20:
		prob20()
	case 21:
		prob21()
	case 22:
		prob22()
	case 23:
		prob23()
	case 24:
		prob24()
	case 25:
		prob25()
	case 26:
		prob26()
	case 27:
		prob27()
	case 28:
		prob28()
	case 29:
		prob29()
	case 30:
		prob30()
	case 31:
		prob31()
	case 32:
		prob32()
	case 33:
		prob33()
	case 34:
		prob34()
	case 35:
		prob35()
	case 36:
		prob36()
	case 37:
		prob37()
	case 38:
		prob38()
	case 39:
		prob39()
	case 40:
		prob40()
	case 41:
		prob41()
	case 42:
		prob42()
	case 43:
		prob43()
	case 44:
		prob44()
	case 45:
		prob45()
	case 46:
		prob46()
	case 47:
		prob47()
	case 48:
		prob48()
	case 49:
		prob49()
	case 50:
		prob50()
	case 52:
		prob52()
	case 53:
		prob53()
	case 55:
		prob55()
	case 56:
		prob56()
	case 57:
		prob57()
	case 58:
		prob58()
	case 60:
		prob60()
	case 62:
		prob62()
	case 63:
		prob63()
	case 64:
		prob64()
	case 65:
		prob65()
	default:
		fmt.Println("not solved")
	}
}
