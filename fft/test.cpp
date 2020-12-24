#include <spqlios-fft.h>
#include <stdio.h>
#include <mkl_dfti.h>

const int N = 16;

void init(double *r)
{
	for (int i = 0; i < N; i++) {
		r[i] = i + 1;
	}
}

void put(const double *r, const char *msg = 0)
{
	if (msg) printf("%s\n", msg);
	for (int i = 0; i < N; i++) {
		printf("%8.4f", r[i]);
		if ((i % 4) == 3) printf("\n");
	}
}

void spqlios()
{
	FFT_Processor_Spqlios spq(N);
	double r[N];
	init(r);
	put(r, "init");
	const double c = 2 / double(N);
	for (int i = 0; i < N; i++) {
		spq.real_inout_direct[i] = r[i];// * c;
	}
	fft(spq.tables_direct, spq.real_inout_direct);
	put(spq.real_inout_direct, "fft");
	for (int i = 0; i < N; i++) {
		spq.real_inout_rev[i] = spq.real_inout_direct[i];
	}
	ifft(spq.tables_reverse, spq.real_inout_rev);
	put(spq.real_inout_rev, "ifft");
}
#if 0
void spqlios()
{
	FFT_Processor_Spqlios spq(N);
	uint32_t a[N];
	uint32_t b[N];
	double r[N];
	init(a);
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
#endif

void fftw3()
{
	double r[N + 2];
	init(r);
	put(r, "init");
	DFTI_DESCRIPTOR_HANDLE hdl, hdl2;
	MKL_LONG status;
	status = DftiCreateDescriptor(&hdl, DFTI_DOUBLE, DFTI_REAL, 1, N);
	if (status) printf("status=%s\n", DftiErrorMessage(status));
	status = DftiCommitDescriptor(hdl);
	if (status) printf("status=%s\n", DftiErrorMessage(status));


	status = DftiComputeForward(hdl, r);
	if (status) printf("status=%s\n", DftiErrorMessage(status));
	put(r, "forward");
	status = DftiComputeBackward(hdl, r);
	if (status) printf("status=%s\n", DftiErrorMessage(status));
	put(r, "backward");
	status = DftiFreeDescriptor(&hdl);
	if (status) printf("status=%s\n", DftiErrorMessage(status));
}

int main()
{
	spqlios();
	fftw3();
}
