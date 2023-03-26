all: gen-lp.exe
CXX?=clang

a.ll: gen_lp.py
	python3 gen_lp.py > a.ll

gen-lp.exe: a.ll lp-test.c
	$(CXX) -o gen-lp.exe a.ll lp-test.c

test: gen-lp.exe
	./gen-lp.exe

clean:
	rm -rf a.ll gen-lp.exe

