#objdump -CS -M intel --no-show-raw-insn ./a.out > a.asm
g++ t.cpp -m32
./a.out
python embed-str.py a.out

python embed-str.py a.out -o emb -s hello
python embed-str.py emb

chmod +x emb
./emb

objdump -D --no-show-raw-insn ./a.out > a.asm
objdump -D --no-show-raw-insn ./emb > b.asm
diff a.asm b.asm
