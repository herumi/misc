PRE=x86_64-w64-mingw32-
CFLAGS=-O2 -DNDEBUG -fno-stack-protector -fno-exceptions -fno-rtti -fno-threadsafe-statics -fPIC
CFLAGS+=-Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wundef
CFLAGS+=-I ./include -I ./src
CFLAGS+=-DMCL_MAX_BIT_SIZE=384
# for dll
CFLAGS+=-DMCL_NO_AUTOLINK -DMCLBN_NO_AUTOLINK -DMCL_DLL_EXPORT

OBJ_DIR=./obj

OBJS=asm.obj sub.obj

all: main.exe

asm.asm: gen.py
	python3 gen.py -win -m nasm > $@
asm.obj: asm.asm
	nasm -fwin64 -o $@ $<

sub.obj: sub.cpp
	$(PRE)$(CXX) -c -o $@ $< $(CFLAGS)

main.obj: main.cpp
	$(PRE)$(CXX) -c -o $@ $< $(CFLAGS)

suba.lib: $(OBJS)
	$(PRE)dlltool -l suba.lib -D suba.dll $(OBJS)

subb.lib: $(OBJS)
	$(PRE)$(AR) r $@ $(OBJS)

bin/suba.dll: suba.lib
	$(PRE)$(LD) -shared -o $@ $(OBJS)

bin/main.exe: main.obj suba.lib
	$(PRE)$(CXX) -o $@ main.obj suba.lib

bin/mainb.exe: main.obj subb.lib
	$(PRE)$(CXX) -o $@ main.obj suba.lib

test: bin/main.exe bin/suba.dll bin/mainb.exe
	wine bin/main.exe
	wine bin/mainb.exe

clean:
	$(RM) $(OBJS) asm.asm *.exe *.obj *.lib *.o *.dll *.exp bin/*.exe bin/*.dll

install:
	sudo apt install mingw-w64 binutils-mingw-w64 gcc-mingw-w64 g++-mingw-w64 wine

#python3 src/gen_bint_x64.py -win -m nasm > $(OBJ_DIR)/bint-x64.asm
#nasm -f win64 -o $(OBJ_DIR)/bint-x64.obj $(OBJ_DIR)/bint-x64.asm
#$(PRE)$(CXX) -c -o $(OBJ_DIR)/fp.obj src/fp.cpp $(CFLAGS)
#$(PRE)$(AR)



