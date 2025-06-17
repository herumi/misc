#define CYBOZU_DONT_USE_OPENSSL
#include <cybozu/sha2.hpp>
#include <mcl/bls12_381.hpp>
#include <cybozu/serializer.hpp>
#include <cybozu/endian.hpp>
#include "bbs.hpp"
#include "bbs.h"
#include "../../mcl/src/cast.hpp"

using namespace mcl;
using namespace bbs;

static const size_t MAX_MSG_SIZE = 1024;
static size_t s_maxMsgSize;
static G1 s_gen[2 + MAX_MSG_SIZE];
static G1& s_Q1 = s_gen[0];
static G1& s_Q2 = s_gen[1];
static G1* s_H = &s_gen[2];
static G1 s_P1;
static G2 s_P2;

static const uint8_t s_dst[] = "MAP_MSG_TO_SCALAR_AS_HASH_";
static const size_t s_dstSize = sizeof(s_dst) - 1;

const int g_ioMode = mcl::IoSerialize;

inline SecretKey *cast(bbsSecretKey *p) { return reinterpret_cast<SecretKey*>(p); }
inline const SecretKey *cast(const bbsSecretKey *p) { return reinterpret_cast<const SecretKey*>(p); }
inline PublicKey *cast(bbsPublicKey *p) { return reinterpret_cast<PublicKey*>(p); }
inline const PublicKey *cast(const bbsPublicKey *p) { return reinterpret_cast<const PublicKey*>(p); }
inline Signature *cast(bbsSignature *p) { return reinterpret_cast<Signature*>(p); }
inline const Signature *cast(const bbsSignature *p) { return reinterpret_cast<const Signature*>(p); }
inline Proof *cast(bbsProof *p) { return reinterpret_cast<Proof*>(p); }
inline const Proof *cast(const bbsProof *p) { return reinterpret_cast<const Proof*>(p); }

mclSize bbsGetSecretKeySerializeByteSize() { return mclBn_getFrByteSize(); }
mclSize bbsGetPublicKeySerializeByteSize() { return mclBn_getG2ByteSize(); }
mclSize bbsGetSignatureSerializeByteSize() { return mclBn_getG1ByteSize() + mclBn_getFrByteSize() * 2; }

mclSize getFixedPartOfProofSerializeByteSize()
{
	return mclBn_getG1ByteSize() * 3 + mclBn_getFrByteSize() * 5 + sizeof(uint32_t);
}
mclSize bbsGetProofSerializeByteSize(const bbsProof *prf)
{
	return getFixedPartOfProofSerializeByteSize() + mclBn_getFrByteSize() * cast(prf)->undiscN;
}

mclSize bbsDeserializeSecretKey(bbsSecretKey *x, const void *buf, mclSize bufSize)
{
	return mclBnFr_deserialize(&x->v, buf, bufSize);
}

mclSize bbsDeserializePublicKey(bbsPublicKey *x, const void *buf, mclSize bufSize)
{
	return mclBnG2_deserialize(&x->v, buf, bufSize);
}

bbsProof* bbsDeserializeProof(const void *buf, mclSize bufSize)
{
	if (bufSize <= getFixedPartOfProofSerializeByteSize()) return 0;
	mclSize n = bufSize - getFixedPartOfProofSerializeByteSize();
	const mclSize FrSize = mclBn_getFrByteSize();
	if (n % FrSize) return 0;
	mclSize undiscN = n / FrSize;
	uint8_t *const top = (uint8_t*)malloc(sizeof(Proof) + sizeof(Fr) * undiscN);
	Proof *prf = (Proof*)top;
	prf->m_hat = (Fr*)(top + sizeof(Proof));

	G1 *G1tbl[] = { &prf->A_prime, &prf->A_bar, &prf->D };
	Fr *Frtbl[] = { &prf->c, &prf->e_hat, &prf->r2_hat, &prf->r3_hat, &prf->s_hat };

	cybozu::MemoryInputStream is(buf, bufSize);
	bool b = false;
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(G1tbl); i++) {
		G1tbl[i]->load(&b, is, g_ioMode);
		if (!b) goto ERR;
	}
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(Frtbl); i++) {
		Frtbl[i]->load(&b, is, g_ioMode);
		if (!b) goto ERR;
	}
	// load undiscN
	{
		const size_t an = sizeof(uint32_t);
		uint8_t a[4];
		if (is.readSome(a, an) != an) goto ERR;
		uint32_t v = cybozu::Get32bitAsLE(a);
		if (v != undiscN) goto ERR;
		prf->undiscN = v;
	}
	// load m_hat[undiscN]
	for (size_t i = 0; i < undiscN; i++) {
		prf->m_hat[i].load(&b, is, g_ioMode);
		if (!b) goto ERR;
	}
	return (bbsProof*)prf;
ERR:;
	free(prf);
	return 0;
}

mclSize bbsSerializeProof(void *buf, mclSize maxBufSize, const bbsProof *x)
{
	cybozu::MemoryOutputStream os(buf, maxBufSize);
	bool b = false;

	const Proof* prf = cast(x);
	const G1 *G1tbl[] = { &prf->A_prime, &prf->A_bar, &prf->D };
	const Fr *Frtbl[] = { &prf->c, &prf->e_hat, &prf->r2_hat, &prf->r3_hat, &prf->s_hat };

	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(G1tbl); i++) {
		G1tbl[i]->save(&b, os, g_ioMode);
		if (!b) return 0;
	}
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(Frtbl); i++) {
		Frtbl[i]->save(&b, os, g_ioMode);
		if (!b) return 0;
	}
	// save undiscN
	{
		const size_t an = sizeof(uint32_t);
		uint8_t a[4];
		cybozu::Set32bitAsLE(a, prf->undiscN);
		os.write(&b, a, an);
		if (!b) return 0;
	}
	// save m_hat[undiscN]
	for (size_t i = 0; i < prf->undiscN; i++) {
		prf->m_hat[i].save(&b, os, g_ioMode);
		if (!b) return 0;
	}
	return os.getPos();
}

mclSize bbsDeserializeSignature(bbsSignature *x, const void *buf, mclSize bufSize)
{
	cybozu::MemoryInputStream is(buf, bufSize);
	bool b;
	cast(&x->A)->load(&b, is, g_ioMode);
	if (!b) return 0;
	cast(&x->e)->load(&b, is, g_ioMode);
	if (!b) return 0;
	cast(&x->s)->load(&b, is, g_ioMode);
	if (!b) return 0;
	return is.getPos();
}

mclSize bbsSerializeSecretKey(void *buf, mclSize maxBufSize, const bbsSecretKey *x)
{
	return mclBnFr_serialize(buf, maxBufSize, &x->v);
}

mclSize bbsSerializePublicKey(void *buf, mclSize maxBufSize, const bbsPublicKey *x)
{
	return mclBnG2_serialize(buf, maxBufSize, &x->v);
}

mclSize bbsSerializeSignature(void *buf, mclSize maxBufSize, const bbsSignature *x)
{
	cybozu::MemoryOutputStream os(buf, maxBufSize);
	bool b;
	cast(&x->A)->save(&b, os, g_ioMode);
	if (!b) return 0;
	cast(&x->e)->save(&b, os, g_ioMode);
	if (!b) return 0;
	cast(&x->s)->save(&b, os, g_ioMode);
	if (!b) return 0;
	return os.getPos();
}

bool bbsIsEqualSecretKey(const bbsSecretKey *lhs, const bbsSecretKey *rhs)
{
	return *cast(&lhs->v) == *cast(&rhs->v);
}

bool bbsIsEqualPublicKey(const bbsPublicKey *lhs, const bbsPublicKey *rhs)
{
	return *cast(&lhs->v) == *cast(&rhs->v);
}

bool bbsIsEqualSignature(const bbsSignature *lhs, const bbsSignature *rhs)
{
	return *cast(&lhs->A) == *cast(&rhs->A) && *cast(&lhs->e) == *cast(&rhs->e) && *cast(&lhs->s) == *cast(&rhs->s);
}

bool bbsIsEqualProof(const bbsProof *lhs, const bbsProof *rhs);

struct Hash {
	cybozu::Sha256 h_;
	template<class T>
	Hash& operator<<(const T& t)
	{
		char buf[sizeof(T)];
		cybozu::MemoryOutputStream os(buf, sizeof(buf));
		cybozu::save(os, t);
		h_.update(buf, os.getPos());
		return *this;
	}
	template<class F>
	void get(F& x)
	{
		uint8_t md[32];
		h_.digest(md, sizeof(md), 0, 0);
		x.setArrayMask(md, sizeof(md));
	}
};

// out = hash(pk, n, s_Q1, s_Q2, s_H[0:n])
inline void calcDom(Fr& out, const G2& pk, size_t n)
{
	Hash hash;
	hash << pk << n << s_Q1 << s_Q2;
	for (size_t i = 0; i < n; i++) {
		hash << s_H[i];
	}
	hash.get(out);
}

// B = s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i]
inline void calcB(G1& B, const Fr& s, const Fr& dom, const Fr *msgs, size_t n)
{
	{
		Fr t[2] = { s, dom };
		G1::mulVec(B, &s_Q1, t, 2); // B = s_Q1 * s + s_Q2 * dom
	}
	G1 T;
	G1::mulVec(T, s_H, msgs, n); // T = sum_i s_H[i] * msgs[i]
	B += T;
	B += s_P1;
}

// write to out[0], ..., out[n-1]
template<class T>
void hash_to_scalar(Fr *out, const T& x, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		Hash hash;
		hash << x << i;
		hash.get(out[i]);
	}
}

// true if all discIdxs[i] < discIdxs[i+1] < msgN
inline bool isValidDiscIdx(size_t msgN, const mclSize *discIdxs, size_t discN)
{
	if (discN == 0) return true;
	if (discIdxs[0] >= msgN) return false;
	for (size_t i = 1; i < discN; i++) {
		if (!(discIdxs[i - 1] < discIdxs[i]) || discIdxs[i] >= msgN) return false;
	}
	return true;
}

// return e(P1, Q) == e(P2, s_P2);
inline bool verifyMultiPairing(const G1& P1, const G1& P2, const G2& Q)
{
	G1 v1[2] = { P1, P2 };
	G2 v2[2] = { Q, -s_P2 };
	GT out;
	millerLoopVec(out, v1, v2, 2);
	finalExp(out, out);
	return out.isOne();
}

inline void msgToFr(Fr& x, const uint8_t *msg, mclSize msgSize)
{
	uint8_t md[64];
	fp::expand_message_xmd(md, sizeof(md), msg, msgSize, s_dst, s_dstSize);
	bool b;
	x.setBigEndianMod(&b, md, sizeof(md));
	assert(b); (void)b;
}

// x: Fr array of size msgN.
// msgs: concatenation of all msg[i]. The size is a sum of msgSize[i].
// msgSize: array of size msgN. msgSize[i] is the size of msg[i].
inline void msgsToFr(Fr *x, const uint8_t *msgs, const mclSize *msgSize, size_t msgN)
{
	for (size_t i = 0; i < msgN; i++) {
		msgToFr(x[i], msgs, msgSize[i]);
		msgs += msgSize[i];
	}
}

void calc_cv(Fr& cv, const Proof& prf, const G1& C1, const G1& C2, size_t discN, const mclSize *discIdxs, const Fr *msgs, bool isDisclosed, const Fr& dom, const uint8_t *nonce = 0, size_t nonceSize = 0)
{
	Hash hash;
	for (size_t i = 0; i < nonceSize; i++) hash << nonce[i];
	hash << prf.A_prime << prf.A_bar << prf.D << C1 << C2 << discN;
	for (size_t i = 0; i < discN; i++) hash << discIdxs[i];
	if (isDisclosed) {
		// disclosed msg
		for (size_t i = 0; i < discN; i++) hash << msgs[i];
	} else {
		// select disclosed msg
		for (size_t i = 0; i < discN; i++) hash << msgs[discIdxs[i]];
	}
	hash << dom;
	hash.get(cv);
}

// out += sum_{i=0}^undiscN s_H[selectedIx[i]] * v[i]
void addSelectedMulVec(G1& out, const mclSize *selectedIdx, size_t undiscN, const Fr *v)
{
	G1 *H = (G1*)CYBOZU_ALLOCA(sizeof(G1) * undiscN);
	for (size_t i = 0; i < undiscN; i++) H[i] = s_H[selectedIdx[i]];
	G1 T;
	G1::mulVec(T, H, v, undiscN);
	out += T;
}

namespace bbs {

namespace local {

void setJs(mclSize *js, size_t undiscN, const mclSize *discIdxs, size_t discN)
{
	const size_t msgN = undiscN + discN;
	size_t v = 0;
	size_t dPos = 0;
	mclSize next = dPos < discN ? discIdxs[dPos++]: msgN;

	size_t jPos = 0;
	while (jPos < undiscN) {
		if (v < next) {
			js[jPos++] = v;
		} else {
			next = dPos < discN ? discIdxs[dPos++]: msgN;
		}
		v++;
	}
}

} // bbs::local

bool init(size_t maxMsgSize)
{
	if (maxMsgSize > MAX_MSG_SIZE) {
		return false;
	}
	s_maxMsgSize = maxMsgSize;
	initPairing(BLS12_381);
	Fp::setETHserialization(true);
	Fr::setETHserialization(true);
	setMapToMode(MCL_MAP_TO_MODE_HASH_TO_CURVE);
	// setup generator
	mapToG1(s_P1, -1);
	mapToG2(s_P2, -2);
	s_P1.normalize();
	s_P2.normalize();

	mapToG1(s_Q1, 1);
	mapToG1(s_Q2, 2);
	s_Q1.normalize();
	s_Q2.normalize();
	for (size_t i = 0; i < maxMsgSize; i++) {
		mapToG1(s_H[i], 3 + i);
		s_H[i].normalize();
	}
	return true;
}

void SecretKey::init()
{
	cast(&v.v)->setByCSPRNG();
}

void SecretKey::getPublicKey(PublicKey& pub) const
{
	G2::mul(*cast(&pub.v.v), s_P2, *cast(&v.v));
}

const Fr& SecretKey::get_v() const
{
	return *cast(&v.v);
}

const G2& PublicKey::get_v() const
{
	return *cast(&v.v);
}

const G1& Signature::get_A() const
{
	return *cast(&v.A);
}

const Fr& Signature::get_e() const
{
	return *cast(&v.e);
}

const Fr& Signature::get_s() const
{
	return *cast(&v.s);
}

/*
	e, s: generated from msgs
	B = s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i]
	A = (1/(sec + e))B
	return (A, e, s)
	assume msgN <= s_maxMsgSize
*/
bool Signature::sign(const SecretKey& sec, const PublicKey& pub, const uint8_t *msgs, const mclSize *msgSize, size_t msgN)
{
	if (msgN > s_maxMsgSize) {
		return false;
	}
	Fr *xs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * msgN);
	msgsToFr(xs, msgs, msgSize, msgN);

	G1& A = *cast(&v.A);
	Fr& e = *cast(&v.e);
	Fr& s = *cast(&v.s);

	Fr dom;
	calcDom(dom, pub.get_v(), msgN);
	Hash hash;
	hash << sec.get_v() << dom;
	for (size_t i = 0; i < msgN; i++) {
		hash << xs[i];
	}
	Fr t;
	hash.get(t);
	Fr out[2];
	hash_to_scalar(out, t, 2);
	e = out[0];
	s = out[1];
	G1 B;
	calcB(B, s, dom, xs, msgN);
	Fr::add(t, sec.get_v(), e);
	Fr::inv(t, t);
	G1::mul(A, B, t);
	return true;
}

/*
	dom = hash(pk, n, s_Q1, s_Q2, s_H[0:n])
	e(A, pub + e * s_P2) = e(s_P1 + s_Q1 * s + s_Q2 * dom + sum_i s_H[i] * msgs[i], s_P2)
*/
bool Signature::verify(const PublicKey& pub, const uint8_t *msgs, const mclSize *msgSize, size_t msgN) const
{
	if (msgN > s_maxMsgSize) return false;

	const G1& A = get_A();
	const Fr& e = get_e();
	const Fr& s = get_s();

	Fr *xs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * msgN);
	msgsToFr(xs, msgs, msgSize, msgN);

	Fr dom;
	calcDom(dom, pub.get_v(), msgN);
	G1 B;
	calcB(B, s, dom, xs, msgN);
	return verifyMultiPairing(A, B, pub.get_v() + s_P2 * e);
}

bool proofGen(Proof& prf, const PublicKey& pub, const Signature& sig, const uint8_t *msgs, const mclSize *msgSize, size_t msgN, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize)
{
	if (msgN > s_maxMsgSize) return false;
	if (msgN < discN) return false;
	if (prf.undiscN != msgN - discN) return false;
	if (!isValidDiscIdx(msgN, discIdxs, discN)) return false;

	Fr *xs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * msgN);
	msgsToFr(xs, msgs, msgSize, msgN);

	Fr dom;
	calcDom(dom, pub.get_v(), msgN);
	Fr out[6];
	hash_to_scalar(out, 0, 6);
	const Fr& r1 = out[0];
	const Fr& r2 = out[1];
	const Fr& e_tilde = out[2];
	const Fr& r2_tilde = out[3];
	const Fr& r3_tilde = out[4];
	const Fr& s_tilde = out[5];
	G1 B;
	calcB(B, sig.get_s(), dom, xs, msgN);
	Fr r3;
	Fr::inv(r3, r1);
	prf.A_prime = sig.get_A() * r1;
	prf.A_bar = prf.A_prime * (-sig.get_e()) + B * r1;
	prf.D = B * r1 + s_Q1 * r2;
	Fr s_prime;
	s_prime = r2 * r3 + sig.get_s();
	G1 C1, C2;
	C1 = prf.A_prime * e_tilde + s_Q1 * r2_tilde;
	C2 = prf.D * (-r3_tilde) + s_Q1 * s_tilde;

	mclSize *js = (mclSize*)CYBOZU_ALLOCA(sizeof(mclSize) * prf.undiscN);
	local::setJs(js, prf.undiscN, discIdxs, discN);
	Fr *m_tilde = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * prf.undiscN);
	hash_to_scalar(m_tilde, "m_tilde", prf.undiscN);
	if (prf.undiscN > 0) {
		addSelectedMulVec(C2, js, prf.undiscN, m_tilde);
	}
	calc_cv(prf.c, prf, C1, C2, discN, discIdxs, xs, false, dom, nonce, nonceSize);
	prf.e_hat= prf.c * sig.get_e() + e_tilde;
	prf.r2_hat = prf.c * r2 + r2_tilde;
	prf.r3_hat = prf.c * r3 + r3_tilde;
	prf.s_hat = prf.c * s_prime + s_tilde;

	if (prf.undiscN > 0) {
		for (size_t i = 0; i < prf.undiscN; i++) {
			prf.m_hat[i] = prf.c * xs[js[i]] + m_tilde[i];
		}
	}
	return true;
}

bool proofVerify(const PublicKey& pub, const Proof& prf, size_t msgN, const uint8_t *discMsgs, const mclSize *discMsgSize, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize)
{
	if (msgN > s_maxMsgSize) return false;
	if (msgN < discN) return false;
	if (prf.undiscN != msgN - discN) return false;
	if (!isValidDiscIdx(msgN, discIdxs, discN)) return false;

	Fr *xs = (Fr*)CYBOZU_ALLOCA(sizeof(Fr) * discN);
	msgsToFr(xs, discMsgs, discMsgSize, discN);

	Fr dom;
	calcDom(dom, pub.get_v(), msgN);
	G1 C1 = (prf.A_bar - prf.D) * prf.c + prf.A_prime * prf.e_hat + s_Q1 * prf.r2_hat;
	G1 T = s_P1 + s_Q2 * dom;
	if (discN > 0) {
		addSelectedMulVec(T, discIdxs, discN, xs);
	}
	G1 C2 = T * prf.c - prf.D * prf.r3_hat + s_Q1 * prf.s_hat;
	if (prf.undiscN > 0) {
		mclSize *js = (mclSize*)CYBOZU_ALLOCA(sizeof(mclSize) * prf.undiscN);
		local::setJs(js, prf.undiscN, discIdxs, discN);
		addSelectedMulVec(C2, js, prf.undiscN, prf.m_hat);
	}
	Fr cv;
	calc_cv(cv, prf, C1, C2, discN, discIdxs, xs, true, dom, nonce, nonceSize);
	if (cv != prf.c) return false;
	if (prf.A_prime.isZero()) return false;
	return verifyMultiPairing(prf.A_prime, prf.A_bar, pub.get_v());
}

} // bbs

bool bbsInit(mclSize maxMsgSize)
{
	return bbs::init(maxMsgSize);
}

bool bbsInitSecretKey(bbsSecretKey *sec)
{
	cast(sec)->init();
	return true;
}

bool bbsGetPublicKey(bbsPublicKey *pub, const bbsSecretKey *sec)
{
	cast(sec)->getPublicKey(*cast(pub));
	return true;
}

bool bbsSign(bbsSignature *sig, const bbsSecretKey *sec, const bbsPublicKey *pub, const uint8_t *msgs, const mclSize *msgSize, mclSize msgN)
{
	return cast(sig)->sign(*cast(sec), *cast(pub), msgs, msgSize, msgN);
}

bool bbsVerify(const bbsSignature *sig, const bbsPublicKey *pub, const uint8_t *msgs, const mclSize *msgSize, mclSize msgN)
{
	return cast(sig)->verify(*cast(pub), msgs, msgSize, msgN);
}

bbsProof *bbsCreateProof(const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *msgs, const mclSize *msgSize, size_t msgN, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize)
{
	if (msgN < discN) return 0;
	const mclSize undiscN = msgN - discN;
	uint8_t *p = (uint8_t*)malloc(sizeof(bbs::Proof) + sizeof(mcl::Fr) * undiscN);
	if (p == 0) return 0;
	bbsProof *proof = (bbsProof*)p;
	mcl::Fr *fr = (mcl::Fr*)(p + sizeof(bbs::Proof));
	cast(proof)->set(fr, undiscN);
	if (bbs::proofGen(*cast(proof), *cast(pub), *cast(sig), msgs, msgSize, msgN, discIdxs, discN, nonce, nonceSize)) {
		return proof;
	}
	free(p);
	return 0;
}

void bbsDestroyProof(bbsProof *proof)
{
	free(proof);
}

bool bbsVerifyProof(const bbsPublicKey *pub, const bbsProof *prf, size_t msgN, const uint8_t *discMsgs, const mclSize *discMsgSize, const mclSize *discIdxs, size_t discN, const uint8_t *nonce, size_t nonceSize)
{
	return bbs::proofVerify(*cast(pub), *cast(prf), msgN, discMsgs, discMsgSize, discIdxs, discN, nonce, nonceSize);
}

