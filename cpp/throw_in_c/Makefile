CFLAGS=-O2 -Wall -Wextra
CXXFLAGS=$(CFLAGS)

.cpp: .o
	$(CXX) -c $< $(CXXFLAGS)

.c: .o
	$(CC) -c $< $(CFLAGS)

t1: t.o s.o
	$(CXX) -o t1 t.o s.o $(LDFLAGS)

test1: t1
	./t1

t2: u.o v.o w.o
	$(CXX) -o t2 u.o v.o w.o $(LDFLAGS)

test1: t1
	./t1

test2: t2
	./t2

clean:
	$(RM) -rf *.o t1 t2
