all: test
CFLAGS=-DNDEBUG -Ofast -I ./inc

pre: inc/t.h
	$(CXX) $< $(CFLAGS) -H

test: t.cpp
	time $(CXX) -o $@ $< $(CFLAGS)

clean:
	-$(RM) t.h.gch test
