TARGET=exception_c.js

all: $(TARGET)

CFLAGS=-O2 -DNDEUBG -Wall -Wextra

EMCC_OPT+=-s WASM=1
#EMCC_OPT+=-s NO_EXIT_RUNTIME=1
#EMCC_OPT+=-s NODEJS_CATCH_EXIT=0
#EMCC_OPT+=-s NODEJS_CATCH_REJECTION=0
EMCC_OPT+=-s MODULARIZE=1
EMCC_OPT+=-s SINGLE_FILE=1
EMCC_OPT+=-s ABORTING_MALLOC=0

# MODE = 1 and MODE = 2 makes the same binary
ifeq ($(MODE),1)
  # catch C++ exception in JS
  EMCC_OPT+=-sNO_DISABLE_EXCEPTION_CATCHING
endif
ifeq ($(MODE),2)
  # catch C++ exception in JS
  EMCC_OPT+=-fexceptions
endif
exception_c.js: exception.cpp
	emcc -o $@ $< $(EMCC_OPT)


test: $(TARGET)
	node test.js

clean:
	$(RM) -rf $(TARGET)
