all: add_c.js

EMCC_OPT=-O3 -DNDEBUG
EMCC_OPT+=-s WASM=1 -s NO_EXIT_RUNTIME=1 -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0
EMCC_OPT+=-s MODULARIZE=1
EMCC_OPT+=-s EXPORT_NAME='createModule'
EMCC_OPT+=-s STRICT_JS=1
EMCC_OPT+=-s SINGLE_FILE=1
EMCC_OPT+=-s MINIFY_HTML=0
EMCC_OPT+=-s ABORTING_MALLOC=0
EMCC_OPT+=-s FILESYSTEM=0
EMCC_OPT+=--minify 0

add_c.js: add.cpp Makefile
	emcc -o add_c.js add.cpp $(EMCC_OPT)

test: add.js add_c.js
	node test.js

clean:
	rm -rf add_c.js
