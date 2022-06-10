SRC=a.cpp b.cpp m.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)

TARGET=m.exe
all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

$(TARGET): $(OBJ)
	$(CXX) $< -o $@ $(LDFLAGS) $(OBJ)

-include $(DEP)


clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)

.PHONY: clean

.SECONDARY: $(OBJ)

