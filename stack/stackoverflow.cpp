#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <signal.h>

#define MAX_STACK_SIZE (4096 * 8)
static char altStack[4096];

static void handler(int sig, siginfo_t *info, void *)
{
	if (sig == SIGSEGV) {
		fprintf(stderr, "SEGV: %p\n", info->si_addr);
		exit(1);
	}
	printf("ERR=%d\n", sig);
}

int f(size_t n)
{
    char buf[n];
    size_t i;
    for (i = 0; i < n; i++) {
        buf[i] = i;
    }
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += buf[i];
    }
    return sum;
}

int main(int argc, char *argv[])
    try
{
	stack_t ss;
	ss.ss_sp = altStack;
	ss.ss_size = sizeof(altStack);
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL)) {
		perror("sigaltstack");
		return 1;
	}
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGSEGV);
	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO | SA_RESTART | SA_ONSTACK;
	if (sigaction(SIGSEGV, &act, NULL) < 0) {
		perror("sigaction");
		return 1;
	}
    argc--, argv++;
    if (argc == 0) {
		puts("set n");
		return 1;
	}
    int n = atoi(argv[0]);
    printf("n=%d\n", n);
    printf("sum=%d\n", f(n));
    return 0;
} catch (std::exception& e) {
    printf("err %s\n", e.what());
}
