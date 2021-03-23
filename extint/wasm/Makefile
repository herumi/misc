LLVM_VER?=-11
ifeq ($(CXX),emcc)
CFLAGS=-O3 -DNDEBUG -DMCL_SIZEOF_UNIT=8 
CFLAGS+=-s WASM=1 -s NO_EXIT_RUNTIME=1 -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0
else
CXX=clang++$(LLVM_VER)
CFLAGS=-O3 -DNDEBUG -fPIC -DMCL_SIZEOF_UNIT=8 -DMCL_MAX_BIT_SIZE=384 -DMCL_LLVM_BMI2=0 -DMCL_USE_LLVM=1 -DCYBOZU_DONT_USE_EXCEPTION -DCYBOZU_MINIMUM_EXCEPTION -DCYBOZU_DONT_USE_STRING -DMCL_DONT_USE_CSPRNG -I./include -I./src -I../mcl/include -fno-builtin  --target=wasm32-unknown-unknown-wasm  -Wstrict-prototypes -Wno-unused-parameter -ffreestanding -fno-exceptions -fvisibility=hidden -Wall -Wextra -fno-threadsafe-statics -nodefaultlibs -nostdlib -fno-use-cxa-atexit -fno-unwind-tables -fno-rtti -nostdinc++ -DLLONG_MIN=-0x8000000000000000LL
endif
CFLAGS+=-I ../../../mcl/

# apt install lld for wasm-ld

addsub.wasm: add.o sub.o #multi3.o
#	wasm-ld$(LLVM_VER) --no-entry --export-dynamic --initial-memory=131072 -o $@ $?
	wasm-ld$(LLVM_VER) --no-entry --export-dynamic -o $@ $? --allow-undefined
#	wasm-ld$(LLVM_VER) --no-entry --export-dynamic -o $@ $? --stack-first
	chmod 644 $@

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

add.o: add.cpp
sub.o: sub.cpp
multi3.o: multi3.ll
	$(CXX) -c -o $@ $< $(CLANG_WASM_OPT)

dis: add.o
	llvm-objdump$(LLVM_VER) -s -d add.o

add_emcc.o: add.cpp
	emcc -c -O3 -DNDEBUG -I ../../../mcl -o add_emcc.o add.cpp

dis_emcc: add_emcc.o
	llvm-objdump$(LLVM_VER) -s -d add_emcc.o


test: addsub.wasm
	node test.js

clean:
	rm -rf *.o *.wasm
