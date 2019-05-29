all=add.wasm

add.wasm: add.cpp
	emcc -O3 -o add.js -s WASM=1 -s EXPORTED_FUNCTIONS="['_add']" add.cpp

test: add.wasm
	node index.js

test2: add.wasm
	node index2.js

clean:
	rm -rf add.wasm add.js
