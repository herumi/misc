CXX?=clang++
INC_DIR?=../../fujitsu/xbyak_aarch64/
CPPFLAGS=$(OPT) -Wall -I$(INC_DIR) -I ./include #-g

test1.exe: double-vec.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<
test2.exe: int-vec.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<
test3.exe: float-vec.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<
test4.exe: float-test.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<

exp_v: exp_v.cpp fmath-sve.hpp
	$(CXX) $(CPPFLAGS) -o $@ $<

test%: test%.exe
	./$<
emu:
	aarch64-linux-gnu-g++ float-test.cpp -I ../../fujitsu/xbyak_aarch64/
	env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve512=on ./a.out

clean:
	rm -rf test*.exe exp_v
