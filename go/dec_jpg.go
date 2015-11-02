package main

import (
	"fmt"
	"os"
//	"image/jpeg"
	"io/ioutil"
	"path"
	"strings"
)

func ReadDirAll(name string, ss *[]string, suf string) {
	fi, err := os.Stat(name)
	if err != nil {
		fmt.Println("os.Stat", err, name)
		return
	}
	if fi.Mode().IsDir() {
		fis, err := ioutil.ReadDir(name)
		if err != nil {
			fmt.Println("os.ReadDir", err)
			return
		}
		for _, fi := range fis {
			ReadDirAll(path.Join(name, fi.Name()), ss, suf)
		}
		return
	}
	if suf == "" || strings.HasSuffix(name, suf) {
		*ss = append(*ss, name)
	}
}

func main() {
	if len(os.Args) == 1 {
		fmt.Println("dec_jpg (file|dir)")
		return
	}
	var ss []string
	name := os.Args[1]
	ReadDirAll(name, &ss, ".jpg")
	for i, s := range ss {
		fmt.Println(i, s)
	}
}
