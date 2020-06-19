package rand

import (
	"os"
)

func ReadRand() []byte {
	file, err := os.Open("/dev/urandom")
	if err != nil {
		return nil
	}
	defer file.Close()
	buf := make([]byte, 16)
	n, err := file.Read(buf)
	if err != nil {
		return nil
	}
	return buf[:n]
}
