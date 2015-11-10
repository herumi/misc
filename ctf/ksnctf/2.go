package main

import "fmt"

func shift(s string, n int) (r string) {
	for i := 0; i < len(s); i++ {
		c := s[i]
		if c != ' ' {
			if 'a' <= c && c <= 'z' {
				c = byte((int(c)-'a'+n)%26 + 'a')
			} else {
				c = byte((int(c)-'A'+n)%26 + 'A')
			}
		}
		r += string(c)
	}
	return
}

func main() {
	s := "EBG KVVV vf n fvzcyr yrggre fhofgvghgvba pvcure gung ercynprf n yrggre jvgu gur yrggre KVVV yrggref nsgre vg va gur nycunorg. EBG KVVV vf na rknzcyr bs gur Pnrfne pvcure, qrirybcrq va napvrag Ebzr. Synt vf SYNTFjmtkOWFNZdjkkNH. Vafreg na haqrefpber vzzrqvngryl nsgre SYNT"
	fmt.Println(shift(s, 13))
}
