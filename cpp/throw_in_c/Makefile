CFLAGS=-O2 -Wall -Wextra
CXXFLAGS=$(CFLAGS)

t.o: t.cpp
	$(CXX) -c t.cpp $(CXXFLAGS)

s.o: s.c
	$(CC) -c s.c $(CFLAGS)

t: t.o s.o
	$(CXX) -o t t.o s.o $(LDFLAGS)

test: t
	./t

clean:
	$(RM) -rf t.o s.o t
