MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
SRC=bbs_test.cpp
LIB_SRC=bbs.cpp
OBJ=$(SRC:.cpp=.o) $(LIB_SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
TARGET=$(SRC:.cpp=.exe)

all: $(TARGET)

CFLAGS+=-I $(MCL_DIR)/include -I ./ $(CFLAGS_USER)
LDFLAGS+=-L $(MCL_DIR)/lib -L ./ $(LDFLAGS_USER)

libbbs.a: bbs.o
	$(AR) $(ARFLAGS) $@ $<

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.exe: %.o libbbs.a
	$(PRE)$(CXX) $< -o $@ -lmcl -lbbs $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

# wasm is built by plain clang (not emscripten) in the same way as $(MCL_DIR)/Makefile.wasm.
# clang, wasm-ld (lld) and wasm-opt (binaryen) are required.
# $(MCL_DIR)/src/wasm provides a minimal libc, dlmalloc and the template of the JS glue.
LLVM_VER?=-21
WASM_CC=clang$(LLVM_VER)
WASM_CXX=clang++$(LLVM_VER)
WASM_LD=wasm-ld$(LLVM_VER)
WASM_OPT?=wasm-opt
WASM_DIR=$(MCL_DIR)/src/wasm
WASM_OBJ_DIR=./obj/wasm

WASM_TARGET=--target=wasm32-unknown-unknown
WASM_CFLAGS=$(WASM_TARGET) -O3 -DNDEBUG -flto -fvisibility=hidden
WASM_CFLAGS+=-fno-threadsafe-statics -fno-rtti -fno-stack-protector -fno-exceptions
WASM_CFLAGS+=-DMCL_SIZEOF_UNIT=4 -DMCL_STANDALONE
WASM_CFLAGS+=-DMCL_USE_WEB_CRYPTO_API
WASM_CFLAGS+=-DCYBOZU_MINIMUM_EXCEPTION
WASM_CFLAGS+=-I $(WASM_DIR) -I $(MCL_DIR)/include -I $(MCL_DIR)/src -I ./
WASM_CFLAGS+=-Wall -Wextra -MMD -MP
WASM_CXXFLAGS=$(WASM_CFLAGS) -std=c++03

WASM_STACK_SIZE?=1048576
WASM_LDFLAGS=--no-entry --export-dynamic -z stack-size=$(WASM_STACK_SIZE)
WASM_LDFLAGS+=--export=__stack_pointer --export=__wasm_call_ctors
WASM_LDFLAGS+=--export=malloc --export=free
WASM_LDFLAGS+=--export=stackSave --export=stackAlloc --export=stackRestore
WASM_LDFLAGS+=--import-memory --allow-undefined
WASM_LDFLAGS+=--lto-O3

DLMALLOC_FLAGS=-DLACKS_SYS_TYPES_H -DLACKS_FCNTL_H -DLACKS_UNISTD_H
DLMALLOC_FLAGS+=-DLACKS_SYS_MMAN_H -DLACKS_STRINGS_H -DLACKS_SYS_PARAM_H
DLMALLOC_FLAGS+=-DLACKS_SCHED_H -DLACKS_TIME_H
DLMALLOC_FLAGS+=-DHAVE_MORECORE=1 -DHAVE_MMAP=0 -DNO_MALLOC_STATS=1
DLMALLOC_FLAGS+=-DMORECORE_CONTIGUOUS=0
DLMALLOC_FLAGS+=-Dsize_t=unsigned\ long -Dptrdiff_t=long
DLMALLOC_FLAGS+=-DMALLOC_ALIGNMENT=16

WASM_OBJS=$(WASM_OBJ_DIR)/fp.o $(WASM_OBJ_DIR)/bbs.o
WASM_OBJS+=$(WASM_OBJ_DIR)/dlmalloc.o $(WASM_OBJ_DIR)/libc.o $(WASM_OBJ_DIR)/stack.o
WASM_DEPS=$(WASM_OBJS:.o=.d)

BBS_WASM=$(WASM_OBJ_DIR)/bbs.wasm
BBS_JS=./wasm/bbs_c.js

$(BBS_JS): $(BBS_WASM) $(WASM_DIR)/glue.js
	python3 -c "import base64;w=base64.b64encode(open('$(BBS_WASM)','rb').read()).decode();print(open('$(WASM_DIR)/glue.js').read().replace('@@WASM_BASE64@@',w).replace('@@STACK_SIZE@@','$(WASM_STACK_SIZE)'),end='')" > $@

$(BBS_WASM): $(WASM_OBJS)
	$(WASM_LD) -o $@.pre $(WASM_OBJS) $(WASM_LDFLAGS)
	$(WASM_OPT) -O3 --strip-debug $@.pre -o $@
	rm $@.pre

# do not export mclBn* functions in fp.cpp (only bbs* are exported)
$(WASM_OBJ_DIR)/fp.o: $(MCL_DIR)/src/fp.cpp | $(WASM_OBJ_DIR)
	$(WASM_CXX) $(WASM_CXXFLAGS) -DMCL_DONT_EXPORT -c -o $@ $<

$(WASM_OBJ_DIR)/bbs.o: bbs.cpp | $(WASM_OBJ_DIR)
	$(WASM_CXX) $(WASM_CXXFLAGS) -c -o $@ $<

$(WASM_OBJ_DIR)/dlmalloc.o: $(WASM_DIR)/dlmalloc.c | $(WASM_OBJ_DIR)
	$(WASM_CC) $(WASM_TARGET) -O3 -DNDEBUG -flto -I $(WASM_DIR) $(DLMALLOC_FLAGS) -c -o $@ $< -DUSE_LOCKS=0

$(WASM_OBJ_DIR)/libc.o: $(WASM_DIR)/libc.c | $(WASM_OBJ_DIR)
	$(WASM_CC) $(WASM_TARGET) -O3 -DNDEBUG -flto -I $(WASM_DIR) -c -o $@ $<

$(WASM_OBJ_DIR)/stack.o: $(WASM_DIR)/stack.s | $(WASM_OBJ_DIR)
	$(WASM_CC) $(WASM_TARGET) -c -o $@ $<

$(WASM_OBJ_DIR):
	mkdir -p $@

wasm: $(BBS_JS)

-include $(DEP)
-include $(WASM_DEPS)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP) *.a $(BBS_JS) *.d
	$(RM) $(WASM_OBJS) $(WASM_DEPS) $(BBS_WASM) $(BBS_WASM).pre

.PHONY: clean wasm

# don't remove these files automatically
.SECONDARY: $(OBJ)
