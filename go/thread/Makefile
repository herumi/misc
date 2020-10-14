CFLAGS=-Wall -Wextra -O3 -I./include -std=c++11 -pthread

wait.o: wait.cpp
	$(CXX) $(CFLAGS) -fPIC -c -o $@ -g $<

main: main.cpp libwait.a
	$(CXX) $(CFLAGS) -o $@ main.cpp libwait.a

libwait.a: wait.o
	ar r libwait.a wait.o

