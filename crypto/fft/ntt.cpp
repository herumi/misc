#ifdef _WIN32
	// for my env. remove this if initPairing shows an error
	#define MCL_MAX_BIT_SIZE 384
#endif
#include <mcl/bls12_381.hpp>

using namespace mcl;
using namespace mcl::bn;

typedef std::vector<Fr> FrVec;

inline void put(const char *msg, const Fr& x)
{
	printf("%s=%s\n", msg, x.getStr(10).c_str());
}

inline void put(const char *msg, const FrVec& x)
{
	printf("%s=", msg);
	for (size_t i = 0; i < x.size(); i++) {
		printf(" %s", x[i].getStr(10).c_str());
	}
	printf("\n");
}

struct NTT {
	int log2N;
	int N;
	Fr g;
	Fr q;
	Fr w;
	Fr invN;
	Fr invW;
	NTT(int log2N)
		: log2N(log2N)
		, N(1 << log2N)
	{
		assert(log2N < 32);
		{
			Vint t;
			Vint::invMod(t, N, Fr::getOp().mp);
			invN.setMpz(t);
		}
		q = -1;
		q /= N;
		uint32_t root = 5;
		Fr::pow(g, root, N);
		Fr::pow(w, root, q);
		Fr::inv(invW, w);
		put("invN", invN);
		put("g", g);
		put("w", w);
	}
	template<class T>
	void _fft(std::vector<T>& out, const std::vector<T>& in, const Fr& g) const
	{
		out.resize(in.size());
		for (int i = 0; i < N; i++) {
			T& v = out[i];
			v.clear();
			Fr t0;
			Fr::pow(t0, g, i);
			Fr t = 1;
			for (int j = 0; j < N; j++) {
				v += in[j] * t;
				t *= t0;
			}
		}
	}
	template<class T>
	void fft(std::vector<T>& out, const std::vector<T>& in) const
	{
		_fft(out, in, w);
	}
	template<class T>
	void ifft(std::vector<T>& out, const std::vector<T>& in) const
	{
		_fft(out, in, invW);
		for (int i = 0; i < N; i++) {
			out[i] *= invN;
		}
	}
};

int main()
	try
{
	initPairing(mcl::BLS12_381);
	NTT ntt(4);
	FrVec xs(ntt.N), ys, zs;
	for (int i = 0; i < ntt.N; i++) {
		xs[i] = (i + 1) * 12345;
	}
	put("xs", xs);
	ntt.fft(ys, xs);
	put("ys", ys);
	ntt.ifft(zs, ys);
	put("zs", zs);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
