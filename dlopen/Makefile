test1:
	@echo --- test1 ---
	$(CC) main.c sub.c && ./a.out

test2:
	@echo --- test2 ---
	$(CC) main.c sub.c -fsanitize=address && ./a.out

test3:
	@echo --- test3 ---
	$(CC) -fPIC -shared sub.c -o sub.so
	$(CC) main.c ./sub.so && ./a.out

test4:
	@echo --- test4 ---
	$(CC) -fPIC -shared sub.c -o sub.so -fsanitize=address
	$(CC) main.c ./sub.so -lasan
	-./a.out

test5:
	@echo --- test5 ---
	$(CC) -fPIC -shared sub.c -o sub.so -fsanitize=address
	$(CC) main.c ./sub.so -lasan && env LD_PRELOAD=/usr/lib/gcc/x86_64-linux-gnu/7/libasan.so ./a.out

test6:
	@echo --- test6 ---
	$(CC) -fPIC -shared sub.c -o sub.so
	$(CC) main-dl.c -ldl
	./a.out

test7:
	@echo --- test7 ---
	$(CC) -fPIC -shared sub.c -o sub.so -fsanitize=address
	$(CC) main-dl.c -ldl -fsanitize=address
	./a.out

test8:
	@echo --- test8 ---
	$(CC) -fPIC -shared sub.c -o sub.so -fsanitize=address
	$(CC) main-dl.c -ldl -fsanitize=address -DDEEP
	./a.out

test:
	$(MAKE) test1 test2 test3 test4 test5 test6 test7
