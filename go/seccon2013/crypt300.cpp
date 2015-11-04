#include <mcl/ec.hpp>
#include <mcl/fp.hpp>

typedef mcl::FpT<> Fp;
struct tagZn;
typedef mcl::FpT<tagZn> Zn;
typedef mcl::EcT<Fp> Ec;

// return n such that nP == Q
int GetDLP(const Ec& P, const Ec& Q)
{
	Ec R(P);
	for (int i = 1; i < 10000000; i++) {
		if (R == Q) {
			return i;
		}
		R += P;
	}
	throw cybozu::Exception("GetDLP") << P << Q;
}
int main()
	try
{
	Fp::setModulo("7654319", 10, mcl::fp::FP_GMP);
	Zn::setModulo("7654319");
	Ec::setParam("1234577", "3213242");
	const Ec base(Fp("5234568"), Fp("2287747"));
	const Ec Pub(Fp("2366653"), Fp("1424308"));
	const Ec O1(Fp("5081741"), Fp("6744615"));
	const Ec O2(Fp("610619"), Fp("6218"));
	Ec R(base);
	const int secretKey = GetDLP(base, Pub);
	printf("secretKey=%d\n", secretKey);
	const int r = GetDLP(base, O1);
	printf("r=%d\n", r);
	Ec plain;
	Ec::mul(plain, Pub, r);
	Ec::sub(plain, O2, plain);
	plain.normalize();
	std::cout << plain.x + plain.y << std::endl;
} catch (std::exception& e) {
	printf("err=%s\n", e.what());
	return 1;
}
