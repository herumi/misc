
%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

main1.exe: main.o sub1.o sub2.o
	$(PRE)$(CXX) $^ -o $@ $(LDFLAGS)

main2.exe: main.o sub2.o sub1.o
	$(PRE)$(CXX) $^ -o $@ $(LDFLAGS)

main3.exe: sub1.o sub2.o main.o
	$(PRE)$(CXX) $^ -o $@ $(LDFLAGS)

main4.exe: sub2.o sub1.o main.o
	$(PRE)$(CXX) $^ -o $@ $(LDFLAGS)

test: main1.exe main2.exe main3.exe main4.exe
	./main1.exe
	./main2.exe
	./main3.exe
	./main4.exe

clean:
	\rm -rf *.o *.exe
