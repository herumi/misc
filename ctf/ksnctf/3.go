/*
view-source:http://ksnctf.sweetduet.info/q/3/unya.htmlのソースを開く
13行目の(ᒧᆞωᆞ)=(/ᆞωᆞ/),(ᒧᆞωᆞ).ᒧ...
をコピペしてコンソールに張り付ける。
$ is not definedとでるのでdebuggerを開く
"ᒧᆞωᆞ"の"ᒧうーｰｰｰー"に$(function()...とあるので
p=Array(70,152,195,284,475,612,791,896,810,850,737,1332,1469,1120,1470,832,1785,2196,1520,1480,1449)
を取り出す。
s=""
for (var i = 0; i < p.length; i++) s+=String.fromCharCode(p[i]/(i+1))
*/
package main

import "fmt"

func main() {
	p := []int { 70,152,195,284,475,612,791,896,810,850,737,1332,1469,1120,1470,832,1785,2196,1520,1480,1449 }
	for i := 0; i < len(p); i++ {
		fmt.Printf("%c", p[i] / (i+1))
	}
	fmt.Println("")
}
