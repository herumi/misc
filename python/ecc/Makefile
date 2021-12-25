all: fr.py

fr.py: fp.py
	sed 's/Fp/Fr/g' < fp.py > fr.py

test: fr.py
	python3 fp.py
	python3 fr.py
	python3 ec.py
	python3 ecdh.py
