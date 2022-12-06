CLANG?=clang++-14
SRC=list_err.cpp strlen_err.cpp substr_err.cpp
EXE=$(SRC:cpp=exe)
IGNORE?=0

CFLAGS=-g -fsanitize=memory -fsanitize-memory-track-origins
ifeq ($(IGNORE),1)
	CFLAGS+=-fsanitize-ignorelist=ignorelist.txt
endif

%.exe: %.cpp
	$(CLANG) -o $@ $< $(CFLAGS)

test: $(EXE)
	./list_err.exe
	./strlen_err.exe
	./substr_err.exe

clone_LLVM:
	git clone --depth=1 https://github.com/llvm/llvm-project
	cd llvm-project
	mkdir build
libc: build
	# configure cmake
	cd build ; cmake -GNinja ../llvm \
	-DCMAKE_BUILD_TYPE=Release \
	-DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
	-DCMAKE_C_COMPILER=clang \
	-DCMAKE_CXX_COMPILER=clang++ \
	-DLLVM_USE_SANITIZER=MemoryWithOrigins
	# build the libraries
	cd build ; cmake --build . -- cxx cxxabi

clean:
	rm -f $(EXE)
