zig fmt *.zig
zig build-exe main.zig -I ../../bls/include/ -I ../../bls/mcl/include/ -L ../../bls-eth-go-binary/bls/lib/linux/amd64/ -l bls384_256 -l stdc++ -femit-bin=bls.exe && ./bls.exe
#
#zig fmt build.zig.zon
#zig build --summary all
#zig build-exe bls-test.zig -I ../../bls/include/ -I ../../bls/mcl/include/ -L ../../bls-eth-go-binary/bls/lib/linux/amd64/ -l bls384_256 -l stdc++ && ./bls-test
#zig test bls-test.zig -I ../../bls/include/ -I ../../bls/mcl/include/ -L ../../bls-eth-go-binary/bls/lib/linux/amd64/ -l bls384_256 -l stdc++
