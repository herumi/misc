package rand
import (
	"fmt"
	"testing"
	"crypto/rand"
)

func TestRand(t *testing.T) {
	buf := ReadRand()
	if buf == nil {
		t.Fatalf("ReadRand err")
	}
	fmt.Printf("buf=%v\n", buf)

	n, err := rand.Read(buf)
	if err != nil {
		t.Fatalf("rand.Read err")
	}
	fmt.Printf("buf2=%v\n", buf[:n])
}
