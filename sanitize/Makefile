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

clean:
	rm -f $(EXE)
