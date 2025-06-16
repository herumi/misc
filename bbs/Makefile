MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
SRC=bbs_test.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
TARGET=$(SRC:.cpp=.exe)

all: $(TARGET)

CFLAGS+=-I $(MCL_DIR)/include -I ./
LDFLAGS+=-L $(MCL_DIR)/lib -L ./

libbbs.a: bbs.o
	$(AR) $(ARFLAGS) $@ $<

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.exe: %.o libbbs.a
	$(PRE)$(CXX) $< -o $@ -lmcl -lbbs $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

BBS_JS=./bbs_c.js
EMCC_OPT=-I $(MCL_DIR)/include -I $(MCL_DIR)/src -Wall -Wextra -I ./
EMCC_OPT+=-O3 -DNDEBUG -std=c++03
EMCC_OPT+=-fno-threadsafe-statics -fno-rtti
#EMCC_OPT+=-fno-rtti -fno-stack-protector -fno-exceptions -DCYBOZU_DONT_USE_EXCEPTION -DCYBOZU_DONT_USE_STRING
EMCC_OPT+=-DMCL_SIZEOF_UNIT=4
EMCC_OPT+=-s WASM=1 -s NO_EXIT_RUNTIME=1 -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0
EMCC_OPT+=-s MODULARIZE=1
#EMCC_OPT+=-s STRICT_JS=1
EMCC_OPT+=-s SINGLE_FILE=1
EMCC_OPT+=--minify 0
EMCC_OPT+=-DCYBOZU_MINIMUM_EXCEPTION
EMCC_OPT+=-s ABORTING_MALLOC=0
EMCC_OPT+=-s DISABLE_EXCEPTION_CATCHING=1
EMCC_OPT+=-s STACK_SIZE=1MB
EMCC_OPT+=-sEXPORTED_FUNCTIONS=_malloc,_free,stackAlloc,stackSave,stackRestore
EMCC_OPT+=-sEXPORTED_RUNTIME_METHODS=HEAP8,HEAP32
#EMCC_OPT+=-msimd128
EMCC_OPT+=-flto
$(BBS_JS):
	emcc -o $@ $(MCL_DIR)/src/fp.cpp dep.cpp bbs.cpp $(EMCC_OPT) -DMCL_MAX_BIT_SIZE=384 -DMCL_USE_WEB_CRYPTO_API
	# disable require fs, path
	perl -i -pe 's@(.* = require\(.*)@//\1@' $@

wasm: $(BBS_JS)

-include $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP) *.a $(BBS_JS)

.PHONY: clean

# don't remove these files automatically
.SECONDARY: $(OBJ)
