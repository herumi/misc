
PREFIX?=aarch64-linux-gnu-
XBYAK_DIR?=../../fujitsu/xbyak_aarch64
test: emu
	env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve512=on ./emu

emu: test-aarch64.cpp
	$(PREFIX)$(CXX) $(CFLAGS) -o emu test-aarch64.cpp -I $(XBYAK_DIR) -L $(XBYAK_DIR)/lib -lxbyak_aarch64
	
