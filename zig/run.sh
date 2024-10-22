zig fmt bls-test.zig
zig build-exe bls-test.zig -I ../../bls/include/ -I ../../bls/mcl/include/ -L ../../bls-eth-go-binary/bls/lib/linux/amd64/ -l bls384_256 -l stdc++ && ./bls-test
