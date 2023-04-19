test:
	make test1 FILE=gen_ff_x64
	make test1 FILE=gen_bint_x64
test1:
	python3 $(FILE).py -m gas > a.txt
	diff -urN $(FILE)_gas.txt a.txt
	python3 $(FILE).py -win > b.txt
	diff -urN $(FILE)_win.txt b.txt

clean:
	rm -f a.txt b.txt

.PHONY: clean
