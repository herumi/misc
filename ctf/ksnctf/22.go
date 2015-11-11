package main

import (
	"image"
	"image/color"
	"image/draw"
	"image/png"
	"io/ioutil"
	"os"
	"fmt"
)

func main() {
	ifp, _ := os.Open("22.txt")
	defer ifp.Close()
	text, _ := ioutil.ReadAll(ifp)
	fmt.Printf("text=%s\n", string(text))
	w := 31
	h := 31
	black := color.RGBA{0, 0, 0, 255}
	white := color.RGBA{255, 255, 255, 255}
	m := image.NewRGBA(image.Rect(0, 0, w, h))
	draw.Draw(m, m.Bounds(), &image.Uniform{white}, image.ZP, draw.Src)
	x := 0
	y := 0
	for _, c := range string(text) {
		if c == '\r' {
			continue
		}
		if 'A' <= c && c <= 'Z' {
			m.Set(x, y, black)
			fmt.Printf("X")
		} else if 'a' <= c && c <= 'z' {
			fmt.Printf(" ")
		}
		x++
		if x == w + 1 {
			x = 0
			y++
			fmt.Printf("\n")
		}
	}
	ofp, _ := os.Create("22.png")
	defer ofp.Close()
	png.Encode(ofp, m)
}