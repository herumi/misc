CFLAGS=-Wall -Wextra -O3 -I../../cybozulib/include

all: netspeed roundtrip

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

netspeed: netspeed.o
	$(CXX) $(LDFLAGS) $< -o $@

roundtrip: roundtrip.o
	$(CXX) $(LDFLAGS) $< -o $@

test: netspeed
	./netspeed

clean:
	rm -rf netspeed roundtrip
