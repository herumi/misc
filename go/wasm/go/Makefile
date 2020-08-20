R=$(shell go env GOROOT)
main.wasm: main.go
	GOOS=js GOARCH=wasm go build -o main.wasm main.go

test:
	env GOOS=js GOARCH=wasm go run -exec=$(R)/misc/wasm/go_js_wasm_exec main.go

clean:
	$(RM) -rf *.wasm
