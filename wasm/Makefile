all: low_func32.js
ALL_SRC=mul_test.cpp
TARGET=mul_test.exe
all: $(TARGET)

CFLAGS=-O3 -DNDEUBG -Wall -Wextra -I ../../mcl/include
LDFLAGS=-lgmp -lgmpxx

EMCC_OPT=-I ./mcl/include -I ./mcl/src -Wall -Wextra
EMCC_OPT+=-O3 -DNDEBUG
EMCC_OPT+=-s WASM=1 -s NO_EXIT_RUNTIME=1 -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0
EMCC_OPT+=-s MODULARIZE=1
EMCC_OPT+=-s STRICT_JS=1
EMCC_OPT+=-s SINGLE_FILE=1
EMCC_OPT+=--minify 0
EMCC_OPT+=-DCYBOZU_MINIMUM_EXCEPTION
EMCC_OPT+=-s ABORTING_MALLOC=0

low_func32.js: low_func32.hpp low_func32.cpp
	emcc -o $@ low_func32.cpp $(EMCC_OPT) -fno-exceptions

mul.hpp: mk_mul.py
	python3 mk_mul.py > mul.hpp

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.exe: %.o
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS)

test: mul_test.exe
	./mul_test.exe

clean:
	$(RM) -rf *.exe *.o *.d

DEPEND_FILE=$(addsuffix .d,$(basename $(ALL_SRC)))
-include $(DEPEND_FILE)

