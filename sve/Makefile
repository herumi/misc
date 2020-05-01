CXX=clang++
INC_DIR?=../../fujitsu/xbyak_aarch64/
CPPFLAGS=-Wall -g -I$(INC_DIR)

test1.exe: double-vec.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<

test2.exe: int-vec.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<

test1: test1.exe
	./$<
test2: test2.exe
	./$<

clean:
	rm -rf test*.exe
