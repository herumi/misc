SRC=a.cpp b.cpp main.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)

TARGET=main
CFLAGS+=-O3 -Wall -Wextra -DNDEUBG -g

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) -MMD -MP -MF $(@:.o=.d)

-include $(DEP)

$(TARGET): $(OBJ)
	$(CC) -o $@ $? $(LDFLAGS)

clean:
	$(RM) -rf $(OBJ) $(DEP) $(TARGET)

.PHONY: clean

.SECONDARY: $(DEP)
