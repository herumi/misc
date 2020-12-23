#include <spqlios-fft.h>
#include <stdio.h>

int main()
{
	const int N = 16;
	FFT_Processor_Spqlios spq(N);
	uint32_t a[N];
	uint32_t b[N];
	double r[N];
	for (int i = 0; i < N; i++) {
		a[i] = 8;
	}
	spq.execute_reverse_torus32(r, a);
	puts("r");
	for (int i = 0; i < N; i++) {
		printf("%8.4f ", r[i]);
		if ((i % 4) == 3) printf("\n");
	}
	puts("--- inner ---");
	for (int i = 0; i < N; i++) {
		spq.real_inout_rev[i] = a[i];
	}
	ifft(spq.tables_reverse, spq.real_inout_rev);
	for (int i = 0; i < N; i++) {
		printf("%8.4f", spq.real_inout_rev[i]);
		if ((i % 4) == 3) printf("\n");
	}
	puts("--- inner ---");

	puts("b");
	spq.execute_direct_torus32(b, r);
	for (int i = 0; i < N; i++) {
		printf("%d, ", b[i]);
	}
	printf("\n");
	puts("--- outer ---");
	const double c = 2 / double(N);
	for (int i = 0; i < N; i++) {
		spq.real_inout_direct[i] = r[i] * c;
	}
	fft(spq.tables_direct, spq.real_inout_direct);
	for (int i = 0; i < N; i++) {
		printf("%8.4f", spq.real_inout_direct[i]);
		if ((i % 4) == 3) printf("\n");
	}
	puts("--- outer ---");
}
