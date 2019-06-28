CFLAGS=-Wall -Wextra -O3 -I../../cybozulib/include -pthread
LDFLAGS=-lpthread

ALL_SRC=netspeed.cpp roundtrip.cpp
all: netspeed roundtrip

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

netspeed: netspeed.o
	$(CXX) $< -o $@ $(LDFLAGS)

roundtrip: roundtrip.o
	$(CXX) $< -o $@ $(LDFLAGS)

test: netspeed
	./netspeed

clean:
	rm -rf netspeed roundtrip *.o *.d

.SECONDARY: $(ALL_SRC:.cpp=.o)
