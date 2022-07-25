PRE=x86_64-w64-mingw32-
CFLAGS=-O2 -DNDEBUG -fno-stack-protector -fno-exceptions -fno-rtti -fno-threadsafe-statics -fPIC
CFLAGS+=-Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wundef
CFLAGS+=-I ./include -I ./src
CFLAGS+=-DMCL_MAX_BIT_SIZE=384
# for dll
CFLAGS+=-DMCL_NO_AUTOLINK -DMCLBN_NO_AUTOLINK -DMCL_DLL_EXPORT

OBJ_DIR=./obj

OBJS=asm.obj sub.obj main.obj

all: main.exe

asm.asm: gen.py
	python3 gen.py -win -m nasm > $@
asm.obj: asm.asm
	nasm -fwin64 -o $@ $<

sub.obj: sub.cpp
	$(PRE)$(CXX) -c -o $@ $< $(CFLAGS)

main.obj: main.cpp
	$(PRE)$(CXX) -c -o $@ $< $(CFLAGS)

dll.dll: sub.obj asm.obj
	$(PRE)$(LD) -shared -o $@ sub.obj asm.obj

#dll.lib: dll.dll
#	$(PRE)dlltool -l dll.lib dll.dll

main.exe: dll.dll main.obj
	$(PRE)$(CXX) -o $@ main.obj dll.dll

test: main.exe
	wine ./main.exe

#main.exe: $(OBJS)
#	$(PRE)$(CXX) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS) asm.asm main.exe *.obj *.lib *.o

#python3 src/gen_bint_x64.py -win -m nasm > $(OBJ_DIR)/bint-x64.asm
#nasm -f win64 -o $(OBJ_DIR)/bint-x64.obj $(OBJ_DIR)/bint-x64.asm
#$(PRE)$(CXX) -c -o $(OBJ_DIR)/fp.obj src/fp.cpp $(CFLAGS)
#$(PRE)$(AR)



