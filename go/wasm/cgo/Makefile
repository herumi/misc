R=$(shell go env GOROOT)
all: liblib.a

%.o: %.c
	gcc -c -O2 -fPIC -Wall -Wextra -o $@ -g $<

liblib.a: lib.o
	ar r $@ $<

test: liblib.a
	go run main.go

main.wasm: main.go
	GOOS=js GOARCH=wasm go build -o main.wasm main.go

test_go:
	env GOOS=js GOARCH=wasm go run -exec=$(R)/misc/wasm/go_js_wasm_exec main.go

clean:
	rm -rf *.o *.a

