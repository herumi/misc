MCL_JS=./src/sample_c.js

EMCC_OPT+=-s WASM=1 -s NO_EXIT_RUNTIME=1 -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0
EMCC_OPT+=-s MODULARIZE=1
EMCC_OPT+=-s STRICT_JS=1
EMCC_OPT+=-s SINGLE_FILE=1
EMCC_OPT+=-s DISABLE_EXCEPTION_CATCHING=1
EMCC_OPT+=--minify 0

all: $(MCL_JS)

$(MCL_JS): ./src/sample.cpp
	emcc -o $@ src/sample.cpp $(EMCC_OPT) -fno-exceptions -fno-rtti
	# disable require fs, path
	perl -i -pe 's@(.* = require\(.*)@//\1@' $@

clean:
	rm -rf $(MCL_JS)

.PHONY: clean
