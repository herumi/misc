#!/bin/sh
#objdump -CS -M intel --no-show-raw-insn ./a.out > a.asm
PYTHON=python
g++ t.cpp -m32
./a.out t.cpp > a.txt
$PYTHON embed-str.py a.out

$PYTHON embed-str.py a.out -o emb -s hello
$PYTHON embed-str.py emb

chmod +x emb
./emb t.cpp > b.txt

objdump -D --no-show-raw-insn ./a.out > a.asm
objdump -D --no-show-raw-insn ./emb > b.asm
diff a.asm b.asm
diff a.txt b.txt
