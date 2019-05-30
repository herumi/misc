all=add.wasm
VER?=-8
VER_OK?=-7

CPPFLAGS=-O3 -m32 -fno-builtin --target=wasm32-unknown-unknown-wasm -fvisibility=hidden -ffreestanding -fno-exceptions -Wall -Wextra -fno-threadsafe-statics -nodefaultlibs -nostdlib -fno-use-cxa-atexit -fno-unwind-tables -fno-rtti -nostdinc++ 
add.wasm: add.cpp sub.c
	clang++$(VER) -O3 -c -emit-llvm $(CPPFLAGS) add.cpp
	clang$(VER) -O3 -c -emit-llvm $(CPPFLAGS) sub.c
	llvm-link$(VER) -o wasm.bc add.bc sub.bc
	opt$(VER) -O3 wasm.bc -o wasm.bc
	llc$(VER_OK) -O3 -filetype=obj -o wasm.o wasm.bc
	wasm-ld$(VER_OK) --no-entry --no-threads --allow-undefined --strip-all --import-memory -o add.wasm wasm.o 

test: add.wasm
	node index.js

dis: add.wasm
	wasm-dis add.wasm

clean:
	rm -rf add.wasm add.js *.o *.bc
