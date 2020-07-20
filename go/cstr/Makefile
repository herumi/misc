all: libcstr.a

%.o: %.cpp
	$(CXX) -c -O2 -fPIC -Wall -Wextra -o $@ -g $<

libcstr.a: cstr.o
	ar r $@ $<

main: main.cpp libcstr.a
	$(CXX) -o $@ $< -lcstr -L./

test: main libcstr.a
	echo "run main"
	./main
	echo "run cstr.go"
	go run cstr.go
	echo "valgrind"
	go build cstr.go
	valgrind --leak-check=full ./cstr

clean:
	rm -rf *.o *.a main cstr

