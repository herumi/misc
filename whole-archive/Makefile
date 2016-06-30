
CFLAGS=-Wall -Wextra -L./

LIBS=libsub.a libinit_f.a
ng: main.cpp $(LIBS)
	gcc -o $@ $< $(CFLAGS) -lsub -linit_f

ok: main.cpp $(LIBS)
	gcc -o $@ $< $(CFLAGS) -lsub -Wl,--whole-archive libinit_f.a -Wl,--no-whole-archive

libinit_f.a: init_f.o
	ar r $@ $<
libsub.a: sub.o
	ar r $@ $<

sub.o: sub.cpp
init_f.o: init_f.cpp

.o: .cpp
	gcc -c $< -fPIC $(CFLAGS)

test_ng: ng
	./ng

test_ok: ok
	./ok

clean:
	rm -rf *.a *.so *.o ng ok
