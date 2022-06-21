#!/bin/bash

SUF=aarch64-linux-gnu-
${SUF}as $1 -o tmp.o
VALS=($(${SUF}objdump -h tmp.o | awk '/\.text/ { printf("%s %s\n", $3, $6)}'))
SIZE=$((0x${VALS[0]}))
OFFSET=$((0x${VALS[1]}))
echo size=${SIZE}
echo offset=${OFFSET}
dd if=tmp.o of=out.bin ibs=1 skip=${OFFSET} count=${SIZE} status=none
