all: test

t.h.gch: inc/t.h
	$(CXX) -H -o $@ $< -I ./inc

test: t.cpp t.h.gch
	$(CXX) -o $@ $<

clean:
	-$(RM) t.h.gch test
