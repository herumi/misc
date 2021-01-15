#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void handler(int x)
{
	printf("handler x=%d\n", x);
	exit(1);
}

void run()
{
	puts("exec 'push es'");
	// 'push es' is invalid on x64
	__asm__ volatile(".byte 0x06");
	puts("not here");
}

void setHandler()
{
	struct sigaction sa;
	sa.sa_handler = handler;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGILL, &sa, NULL)) {
		perror("sigaction");
		return;
	}
}

void sample()
{
	run();
}

int main()
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}
	if (pid == 0) {
		printf("child %d\n", pid);
		_exit(0);
		setHandler();
	}
	printf("parent %d\n", pid);
	sample();
	run();
	waitpid(pid, NULL, 0);
	puts("parent end");
	return 0;
}
