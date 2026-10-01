/*
	BBS signature (draft-irtf-cfrg-bbs-signatures-12)
	ciphersuite : BLS12-381-SHA-256
*/
#define CYBOZU_DONT_USE_OPENSSL
#include <mcl/bls12_381.hpp>
#include <mcl/array.hpp>
#include <cybozu/endian.hpp>
#include <stdlib.h>
#include <string.h>
#include "bbs.hpp"
#include "bbs.h"
#include "../../mcl/src/cast.hpp"

using namespace mcl;
using namespace bbs;

static const size_t FR_SIZE = 32; // octet_scalar_length
static const size_t G1_SIZE = 48; // octet_point_length
static const size_t G2_SIZE = 96;
static const size_t EXPAND_LEN = 48; // expand_len
static const size_t MAX_DST_SIZE = 255;
static const size_t MAX_KEY_INFO_SIZE = 65535;
static const size_t MIN_KEY_MATERIAL_SIZE = 32;
// Abar, Bbar, D, e^, r1^, r3^, c
static const size_t FIXED_PROOF_SIZE = G1_SIZE * 3 + FR_SIZE * 4;
// r1, r2, e~, r1~, r3~
static const size_t FIXED_RANDOM_SCALAR_N = 5;

#define BBS_CIPHERSUITE_ID "BBS_BLS12381G1_XMD:SHA-256_SSWU_RO_"
// api_id = ciphersuite_id || "H2G_HM2S_"
#define BBS_API_ID BBS_CIPHERSUITE_ID "H2G_HM2S_"

struct Str {
	const char *p;
	size_t size;
};
#define BBS_STR(s) { s, sizeof(s) - 1 }

static const Str s_apiId = BBS_STR(BBS_API_ID);
static const Str s_keyGenDst = BBS_STR(BBS_CIPHERSUITE_ID "KEYGEN_DST_");
static const Str s_h2sDst = BBS_STR(BBS_API_ID "H2S_");
static const Str s_mapDst = BBS_STR(BBS_API_ID "MAP_MSG_TO_SCALAR_AS_HASH_");
static const Str s_seedDst = BBS_STR(BBS_API_ID "SIG_GENERATOR_SEED_");
static const Str s_generatorDst = BBS_STR(BBS_API_ID "SIG_GENERATOR_DST_");
static const Str s_generatorSeed = BBS_STR(BBS_API_ID "MESSAGE_GENERATOR_SEED");

// P1 of BLS12-381-SHA-256
static const char s_P1Hex[] = "a8ce256102840821a3e94ea9025e4662b205762f9776b3a766c872b948f1fd225e7c59698588e70d11406d161b4e28c9";
// the base point of G2 of BLS12-381
static const char s_BP2Hex[] = "93e02b6052719f607dacd3a088274f65596bd0d09920b61ab5da61bbdc7f5049334cf11213945d57e5ac7d055d042b7e024aa2b2f08f0a91260805272dc51051c6e47ad4fa403b02b4510b647ae3d1770bac0326a805bbefd48056c8c121bdb8";

static int s_cipherSuite = -1;
static size_t s_maxMsgN;
// s_gen[0] = Q_1, s_gen[1 + i] = H_(i+1). All points are normalized.
static G1 *s_gen;
// the number of the computed generators
static size_t s_genN;
// the intermediate value v of create_generators to extend s_gen
static uint8_t s_genV[EXPAND_LEN];
static G1 s_P1;
static G2 s_BP2;

inline SecretKey *cast(bbsSecretKey *p) { return reinterpret_cast<SecretKey*>(p); }
inline const SecretKey *cast(const bbsSecretKey *p) { return reinterpret_cast<const SecretKey*>(p); }
inline PublicKey *cast(bbsPublicKey *p) { return reinterpret_cast<PublicKey*>(p); }
inline const PublicKey *cast(const bbsPublicKey *p) { return reinterpret_cast<const PublicKey*>(p); }
inline Signature *cast(bbsSignature *p) { return reinterpret_cast<Signature*>(p); }
inline const Signature *cast(const bbsSignature *p) { return reinterpret_cast<const Signature*>(p); }

// octet string builder
struct Octets {
	// the buffer may hold secret data, so it is cleared before it is released
	Array<uint8_t, true> buf_;
	size_t pos_;
	Octets() : pos_(0) {}
	// allocate maxSize bytes
	bool init(size_t maxSize)
	{
		pos_ = 0;
		return buf_.resize(maxSize);
	}
	const uint8_t *data() const { return buf_.data(); }
	size_t size() const { return pos_; }
	void put(const void *p, size_t n)
	{
		assert(pos_ + n <= buf_.size());
		if (n == 0) return;
		memcpy(buf_.data() + pos_, p, n);
		pos_ += n;
	}
	// I2OSP(v, 8)
	void putInt(uint64_t v)
	{
		uint8_t a[8];
		cybozu::Set64bitAsBE(a, v);
		put(a, sizeof(a));
	}
	template<class T>
	void putT(const T& x, size_t size)
	{
		assert(pos_ + size <= buf_.size());
		size_t n = x.serialize(buf_.data() + pos_, size);
		assert(n == size); (void)n;
		pos_ += size;
	}
	void put(const Fr& x) { putT(x, FR_SIZE); }
	void put(const G1& x) { putT(x, G1_SIZE); }
	void put(const G2& x) { putT(x, G2_SIZE); }
};

inline bool isInitialized()
{
	return s_gen != 0;
}

inline bool isValidMsgN(size_t msgN)
{
	return isInitialized() && msgN <= s_maxMsgN;
}

// x = OS2IP(get_random(expand_len)) mod r. retry if x is zero.
static bool setRandomScalar(Fr& x)
{
	uint8_t buf[EXPAND_LEN];
	for (int i = 0; i < 16; i++) {
		bool b;
		fp::RandGen::get().read(&b, buf, sizeof(buf));
		if (!b) return false;
		x.setBigEndianMod(&b, buf, sizeof(buf));
		secureZero(buf, sizeof(buf));
		if (!b) return false;
		if (!x.isZero()) return true;
	}
	return false;
}

/*
	create_generators of the spec
	extend s_gen to n generators
*/
static bool extendGenerators(size_t n)
{
	if (n <= s_genN) return true;
	G1 *p = (G1*)realloc((void*)s_gen, sizeof(G1) * n);
	if (p == 0) return false;
	s_gen = p;
	if (s_genN == 0) {
		fp::expand_message_xmd(s_genV, EXPAND_LEN, s_generatorSeed.p, s_generatorSeed.size, s_seedDst.p, s_seedDst.size);
	}
	for (size_t i = s_genN; i < n; i++) {
		// v = expand_message(v || I2OSP(i + 1, 8), seed_dst, expand_len)
		uint8_t buf[EXPAND_LEN + 8];
		memcpy(buf, s_genV, EXPAND_LEN);
		cybozu::Set64bitAsBE(buf + EXPAND_LEN, uint64_t(i + 1));
		fp::expand_message_xmd(s_genV, EXPAND_LEN, buf, sizeof(buf), s_seedDst.p, s_seedDst.size);
		hashAndMapToG1(s_gen[i], s_genV, EXPAND_LEN, s_generatorDst.p, s_generatorDst.size);
		s_gen[i].normalize();
	}
	s_genN = n;
	return true;
}

// messages_to_scalars of the spec
// x: Fr array of size msgN.
// msgs: concatenation of all msg[i]. The size is a sum of msgSize[i].
// msgSize: array of size msgN. msgSize[i] is the size of msg[i].
inline void msgsToFr(Fr *x, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN)
{
	for (size_t i = 0; i < msgN; i++) {
		bbs::local::msgToFr(x[i], msgs, msgSize[i]);
		msgs += msgSize[i];
	}
}

/*
	calculate_domain of the spec
	domain = hash_to_scalar(PK || L || Q_1 || H_1 || ... || H_L || api_id || I2OSP(headerSize, 8) || header)
*/
static bool calcDomain(Fr& domain, const G2& pk, size_t L, const uint8_t *header, size_t headerSize)
{
	Octets os;
	if (!os.init(G2_SIZE + 8 + G1_SIZE * (L + 1) + s_apiId.size + 8 + headerSize)) return false;
	os.put(pk);
	os.putInt(L);
	for (size_t i = 0; i < L + 1; i++) {
		os.put(s_gen[i]);
	}
	os.put(s_apiId.p, s_apiId.size);
	os.putInt(headerSize);
	os.put(header, headerSize);
	bbs::local::hashToScalar(domain, os.data(), os.size(), s_h2sDst.p, s_h2sDst.size);
	return true;
}

// B = P1 + Q_1 * v[0] + H_1 * v[1] + ... + H_L * v[L]
// v[0] is domain and v[1 + i] is the scalar of msg[i]
inline void calcB(G1& B, const Fr *v, size_t L)
{
	G1::mulVec(B, s_gen, v, L + 1);
	B += s_P1;
}

// true if all discIdxs[i] < discIdxs[i+1] < msgN
inline bool isValidDiscIdx(size_t msgN, const uint32_t *discIdxs, size_t discN)
{
	if (discN == 0) return true;
	if (discIdxs[0] >= msgN) return false;
	for (size_t i = 1; i < discN; i++) {
		if (!(discIdxs[i - 1] < discIdxs[i]) || discIdxs[i] >= msgN) return false;
	}
	return true;
}

// return e(P1, Q1) * e(P2, Q2) == 1
inline bool isPairingProductOne(const G1& P1, const G2& Q1, const G1& P2, const G2& Q2)
{
	G1 v1[2] = { P1, P2 };
	G2 v2[2] = { Q1, Q2 };
	GT out;
	millerLoopVec(out, v1, v2, 2);
	finalExp(out, out);
	return out.isOne();
}

/*
	ProofChallengeCalculate of the spec
	c = hash_to_scalar(serialize(R, i1, msg_i1, ..., iR, msg_iR, Abar, Bbar, D, T1, T2, domain) || I2OSP(phSize, 8) || ph)
	msgs is the array of the disclosed messages if isDisclosed else the array of all messages
*/
static bool calcChallenge(Fr& c, const G1& Abar, const G1& Bbar, const G1& D, const G1& T1, const G1& T2, const Fr& domain, const uint32_t *discIdxs, size_t discN, const Fr *msgs, bool isDisclosed, const uint8_t *ph, size_t phSize)
{
	Octets os;
	if (!os.init(8 + (8 + FR_SIZE) * discN + G1_SIZE * 5 + FR_SIZE + 8 + phSize)) return false;
	os.putInt(discN);
	for (size_t i = 0; i < discN; i++) {
		os.putInt(discIdxs[i]);
		os.put(isDisclosed ? msgs[i] : msgs[discIdxs[i]]);
	}
	os.put(Abar);
	os.put(Bbar);
	os.put(D);
	os.put(T1);
	os.put(T2);
	os.put(domain);
	os.putInt(phSize);
	os.put(ph, phSize);
	bbs::local::hashToScalar(c, os.data(), os.size(), s_h2sDst.p, s_h2sDst.size);
	return true;
}

// out += sum_{i=0}^{n-1} H_(selectedIdx[i]+1) * v[i]
static bool addSelectedMulVec(G1& out, const uint32_t *selectedIdx, size_t n, const Fr *v)
{
	if (n == 0) return true;
	Array<G1> H;
	if (!H.resize(n)) return false;
	for (size_t i = 0; i < n; i++) H[i] = s_gen[1 + selectedIdx[i]];
	G1 T;
	G1::mulVec(T, H.data(), v, n);
	out += T;
	return true;
}

// deserialize a point of G1 which is not the identity
inline bool getG1(G1& P, const uint8_t *buf)
{
	return P.deserialize(buf, G1_SIZE) == G1_SIZE && !P.isZero();
}

// deserialize a scalar in [1, r-1]
inline bool getFr(Fr& x, const uint8_t *buf)
{
	return x.deserialize(buf, FR_SIZE) == FR_SIZE && !x.isZero();
}

namespace bbs {

namespace local {

void setJs(uint32_t *js, size_t undiscN, const uint32_t *discIdxs, size_t discN)
{
	const size_t msgN = undiscN + discN;
	uint32_t v = 0;
	size_t dPos = 0;
	size_t next = dPos < discN ? discIdxs[dPos++]: msgN;

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

void hashToScalar(Fr& out, const void *msg, size_t msgSize, const void *dst, size_t dstSize)
{
	uint8_t md[EXPAND_LEN];
	fp::expand_message_xmd(md, sizeof(md), msg, msgSize, dst, dstSize);
	bool b;
	out.setBigEndianMod(&b, md, sizeof(md));
	assert(b); (void)b;
	secureZero(md, sizeof(md));
}

void msgToFr(Fr& out, const uint8_t *msg, size_t msgSize)
{
	hashToScalar(out, msg, msgSize, s_mapDst.p, s_mapDst.size);
}

const G1 *getGenerators()
{
	return s_gen;
}

/*
	CoreProofGen of the spec
	(r1, r2, e~, r1~, r3~, m~_1, ..., m~_U) = rs
*/
size_t proofGenWithRandomScalars(uint8_t *proof, size_t maxProofSize, const PublicKey& pub, const Signature& sig, const uint8_t *header, size_t headerSize, const uint8_t *ph, size_t phSize, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN, const uint32_t *discIdxs, size_t discN, const Fr *rs)
{
	if (!isValidMsgN(msgN)) return 0;
	if (discN > msgN) return 0;
	const size_t L = msgN;
	const size_t U = L - discN;
	const size_t proofSize = getProofSize(U);
	if (maxProofSize < proofSize) return 0;
	if (!isValidDiscIdx(L, discIdxs, discN)) return 0;

	const G1& A = sig.get_A();
	const Fr& e = sig.get_e();
	if (A.isZero() || e.isZero() || pub.get_v().isZero()) return 0;

	const Fr& r1 = rs[0];
	const Fr& r2 = rs[1];
	const Fr& e_tilde = rs[2];
	const Fr& r1_tilde = rs[3];
	const Fr& r3_tilde = rs[4];
	const Fr *m_tilde = rs + FIXED_RANDOM_SCALAR_N;
	if (r1.isZero() || r2.isZero()) return 0;

	// v[0] = domain, v[1 + i] = scalar of msg[i]
	// the scalars of the undisclosed messages are secret
	Array<Fr, true> v;
	Array<uint32_t> js;
	if (!v.resize(L + 1) || !js.resize(U)) return 0;
	msgsToFr(v.data() + 1, msgs, msgSize, L);
	if (!calcDomain(v[0], pub.get_v(), L, header, headerSize)) return 0;
	const Fr& domain = v[0];
	const Fr *m = v.data() + 1;
	setJs(js.data(), U, discIdxs, discN);

	// ProofInit
	G1 B;
	calcB(B, v.data(), L);
	G1 D = B * r2;
	G1 Abar = A * (r1 * r2);
	G1 Bbar = D * r1 - Abar * e;
	G1 T1 = Abar * e_tilde + D * r1_tilde;
	G1 T2 = D * r3_tilde;
	if (!addSelectedMulVec(T2, js.data(), U, m_tilde)) return 0;

	Fr c;
	if (!calcChallenge(c, Abar, Bbar, D, T1, T2, domain, discIdxs, discN, m, false, ph, phSize)) return 0;

	// ProofFinalize
	Fr r3;
	Fr::inv(r3, r2);
	const Fr e_hat = e_tilde + e * c;
	const Fr r1_hat = r1_tilde - r1 * c;
	const Fr r3_hat = r3_tilde - r3 * c;

	// proof = (Abar, Bbar, D, e^, r1^, r3^, m^_1, ..., m^_U, c)
	uint8_t *p = proof;
	const G1 *G1tbl[] = { &Abar, &Bbar, &D };
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(G1tbl); i++) {
		if (G1tbl[i]->serialize(p, G1_SIZE) != G1_SIZE) return 0;
		p += G1_SIZE;
	}
	const Fr *Frtbl[] = { &e_hat, &r1_hat, &r3_hat };
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(Frtbl); i++) {
		if (Frtbl[i]->serialize(p, FR_SIZE) != FR_SIZE) return 0;
		p += FR_SIZE;
	}
	for (size_t i = 0; i < U; i++) {
		const Fr m_hat = m_tilde[i] + m[js[i]] * c;
		if (m_hat.serialize(p, FR_SIZE) != FR_SIZE) return 0;
		p += FR_SIZE;
	}
	if (c.serialize(p, FR_SIZE) != FR_SIZE) return 0;
	return proofSize;
}

} // bbs::local

bool init(int cipherSuite, size_t maxMsgN)
{
	if (cipherSuite != BBS_BLS12381_SHA256) return false;
	if (maxMsgN >= 0xffffffff) return false;
	if (s_cipherSuite != cipherSuite) {
		term();
		bool b;
		initPairing(&b, BLS12_381);
		if (!b) return false;
		Fp::setETHserialization(true);
		Fr::setETHserialization(true);
		setMapToMode(MCL_MAP_TO_MODE_HASH_TO_CURVE);
		verifyOrderG1(true);
		verifyOrderG2(true);
		s_P1.setStr(&b, s_P1Hex, IoSerializeHexStr);
		if (!b) return false;
		s_BP2.setStr(&b, s_BP2Hex, IoSerializeHexStr);
		if (!b) return false;
		s_cipherSuite = cipherSuite;
	}
	if (!extendGenerators(maxMsgN + 1)) return false;
	s_maxMsgN = maxMsgN;
	return true;
}

void term()
{
	free(s_gen);
	s_gen = 0;
	s_genN = 0;
	s_maxMsgN = 0;
	s_cipherSuite = -1;
}

bool SecretKey::init()
{
	if (!isInitialized()) return false;
	return setRandomScalar(*cast(&v.v));
}

/*
	KeyGen of the spec
	SK = hash_to_scalar(key_material || I2OSP(length(key_info), 2) || key_info, key_dst)
*/
bool SecretKey::keyGen(const uint8_t *keyMaterial, size_t keyMaterialSize, const uint8_t *keyInfo, size_t keyInfoSize, const uint8_t *keyDst, size_t keyDstSize)
{
	if (!isInitialized()) return false;
	if (keyMaterialSize < MIN_KEY_MATERIAL_SIZE) return false;
	if (keyInfoSize > MAX_KEY_INFO_SIZE) return false;
	if (keyDst == 0) {
		keyDst = (const uint8_t*)s_keyGenDst.p;
		keyDstSize = s_keyGenDst.size;
	}
	if (keyDstSize > MAX_DST_SIZE) return false;
	Octets os;
	if (!os.init(keyMaterialSize + 2 + keyInfoSize)) return false;
	os.put(keyMaterial, keyMaterialSize);
	uint8_t lenBuf[2];
	cybozu::Set16bitAsBE(lenBuf, uint16_t(keyInfoSize));
	os.put(lenBuf, sizeof(lenBuf));
	os.put(keyInfo, keyInfoSize);
	Fr& x = *cast(&v.v);
	bbs::local::hashToScalar(x, os.data(), os.size(), keyDst, keyDstSize);
	return !x.isZero();
}

void SecretKey::getPublicKey(PublicKey& pub) const
{
	G2::mulCT(*cast(&pub.v.v), s_BP2, *cast(&v.v));
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

/*
	CoreSign of the spec
	domain = calculate_domain(PK, Q_1, (H_1, ..., H_L), header, api_id)
	e = hash_to_scalar(serialize((SK, msg_1, ..., msg_L, domain)))
	B = P1 + Q_1 * domain + H_1 * msg_1 + ... + H_L * msg_L
	A = B * (1 / (SK + e))
	return (A, e)
*/
bool Signature::sign(const SecretKey& sec, const PublicKey& pub, const uint8_t *header, size_t headerSize, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN)
{
	if (!isValidMsgN(msgN)) return false;
	const size_t L = msgN;
	if (sec.get_v().isZero()) return false;

	// x[0] = domain, x[1 + i] = scalar of msg[i]
	Array<Fr, true> x;
	if (!x.resize(L + 1)) return false;
	msgsToFr(x.data() + 1, msgs, msgSize, L);
	if (!calcDomain(x[0], pub.get_v(), L, header, headerSize)) return false;

	Fr e;
	{
		Octets os;
		if (!os.init(FR_SIZE * (L + 2))) return false;
		os.put(sec.get_v());
		for (size_t i = 0; i < L; i++) {
			os.put(x[1 + i]);
		}
		os.put(x[0]);
		bbs::local::hashToScalar(e, os.data(), os.size(), s_h2sDst.p, s_h2sDst.size);
	}
	if (e.isZero()) return false;
	G1 B;
	calcB(B, x.data(), L);
	Fr t;
	Fr::add(t, sec.get_v(), e);
	if (t.isZero()) return false;
	Fr::inv(t, t);
	G1 A;
	G1::mulCT(A, B, t);
	secureZero(&t, sizeof(t));
	if (A.isZero()) return false;
	*cast(&v.A) = A;
	*cast(&v.e) = e;
	return true;
}

/*
	CoreVerify of the spec
	B = P1 + Q_1 * domain + H_1 * msg_1 + ... + H_L * msg_L
	e(A, W) * e(A * e - B, BP2) == 1
*/
bool Signature::verify(const PublicKey& pub, const uint8_t *header, size_t headerSize, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN) const
{
	if (!isValidMsgN(msgN)) return false;
	const size_t L = msgN;

	const G1& A = get_A();
	const Fr& e = get_e();
	const G2& W = pub.get_v();
	if (A.isZero() || e.isZero() || W.isZero()) return false;

	// x[0] = domain, x[1 + i] = scalar of msg[i]
	Array<Fr> x;
	if (!x.resize(L + 1)) return false;
	msgsToFr(x.data() + 1, msgs, msgSize, L);
	if (!calcDomain(x[0], W, L, header, headerSize)) return false;

	G1 B;
	calcB(B, x.data(), L);
	G1 T;
	G1::mul(T, A, e);
	T -= B;
	return isPairingProductOne(A, W, T, s_BP2);
}

size_t getProofSize(size_t undiscN)
{
	return FIXED_PROOF_SIZE + FR_SIZE * undiscN;
}

size_t proofGen(uint8_t *proof, size_t maxProofSize, const PublicKey& pub, const Signature& sig, const uint8_t *header, size_t headerSize, const uint8_t *ph, size_t phSize, const uint8_t *msgs, const uint32_t *msgSize, size_t msgN, const uint32_t *discIdxs, size_t discN)
{
	if (!isValidMsgN(msgN)) return 0;
	if (discN > msgN) return 0;
	// calculate_random_scalars(5 + U)
	const size_t rsN = FIXED_RANDOM_SCALAR_N + (msgN - discN);
	Array<Fr, true> rs;
	if (!rs.resize(rsN)) return 0;
	for (size_t i = 0; i < rsN; i++) {
		if (!setRandomScalar(rs[i])) return 0;
	}
	return bbs::local::proofGenWithRandomScalars(proof, maxProofSize, pub, sig, header, headerSize, ph, phSize, msgs, msgSize, msgN, discIdxs, discN, rs.data());
}

/*
	CoreProofVerify of the spec
	(Abar, Bbar, D, e^, r1^, r3^, (m^_1, ..., m^_U), c) = proof
	T1 = Bbar * c + Abar * e^ + D * r1^
	Bv = P1 + Q_1 * domain + sum_{i in disclosed} H_i * msg_i
	T2 = Bv * c + D * r3^ + sum_{j in undisclosed} H_j * m^_j
	c == challenge and e(Abar, W) * e(Bbar, -BP2) == 1
*/
bool proofVerify(const PublicKey& pub, const uint8_t *proof, size_t proofSize, const uint8_t *header, size_t headerSize, const uint8_t *ph, size_t phSize, const uint8_t *discMsgs, const uint32_t *discMsgSize, const uint32_t *discIdxs, size_t discN)
{
	if (!isInitialized()) return false;
	if (proofSize < FIXED_PROOF_SIZE) return false;
	if ((proofSize - FIXED_PROOF_SIZE) % FR_SIZE) return false;
	const size_t U = (proofSize - FIXED_PROOF_SIZE) / FR_SIZE;
	const size_t R = discN;
	// L = U + R <= s_maxMsgN
	if (U > s_maxMsgN || R > s_maxMsgN - U) return false;
	const size_t L = U + R;
	if (!isValidDiscIdx(L, discIdxs, R)) return false;
	const G2& W = pub.get_v();
	if (W.isZero()) return false;

	// octets_to_proof
	G1 Abar, Bbar, D;
	Fr e_hat, r1_hat, r3_hat, c;
	Array<Fr> m_hat;
	if (!m_hat.resize(U)) return false;
	const uint8_t *p = proof;
	G1 *G1tbl[] = { &Abar, &Bbar, &D };
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(G1tbl); i++) {
		if (!getG1(*G1tbl[i], p)) return false;
		p += G1_SIZE;
	}
	Fr *Frtbl[] = { &e_hat, &r1_hat, &r3_hat };
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(Frtbl); i++) {
		if (!getFr(*Frtbl[i], p)) return false;
		p += FR_SIZE;
	}
	for (size_t i = 0; i < U; i++) {
		if (!getFr(m_hat[i], p)) return false;
		p += FR_SIZE;
	}
	if (!getFr(c, p)) return false;

	// v[0] = domain, v[1 + i] = scalar of discMsg[i]
	Array<Fr> v;
	Array<uint32_t> js;
	if (!v.resize(R + 1) || !js.resize(U)) return false;
	msgsToFr(v.data() + 1, discMsgs, discMsgSize, R);
	if (!calcDomain(v[0], W, L, header, headerSize)) return false;
	const Fr& domain = v[0];
	const Fr *m = v.data() + 1;
	bbs::local::setJs(js.data(), U, discIdxs, R);

	// ProofVerifyInit
	G1 T1 = Bbar * c + Abar * e_hat + D * r1_hat;
	G1 Bv = s_P1 + s_gen[0] * domain;
	if (!addSelectedMulVec(Bv, discIdxs, R, m)) return false;
	G1 T2 = Bv * c + D * r3_hat;
	if (!addSelectedMulVec(T2, js.data(), U, m_hat.data())) return false;

	Fr c2;
	if (!calcChallenge(c2, Abar, Bbar, D, T1, T2, domain, discIdxs, R, m, true, ph, phSize)) return false;
	if (c2 != c) return false;
	// e(Abar, W) * e(Bbar, -BP2) = e(Abar, W) * e(-Bbar, BP2)
	G1 negBbar;
	G1::neg(negBbar, Bbar);
	return isPairingProductOne(Abar, W, negBbar, s_BP2);
}

} // bbs

mclSize bbsSizeofSecretKey() { return sizeof(bbsSecretKey); }
mclSize bbsSizeofPublicKey() { return sizeof(bbsPublicKey); }
mclSize bbsSizeofSignature() { return sizeof(bbsSignature); }

mclSize bbsGetSecretKeySerializeByteSize() { return FR_SIZE; }
mclSize bbsGetPublicKeySerializeByteSize() { return G2_SIZE; }
mclSize bbsGetSignatureSerializeByteSize() { return G1_SIZE + FR_SIZE; }
mclSize bbsGetProofSize(uint32_t undiscN) { return bbs::getProofSize(undiscN); }

mclSize bbsDeserializeSecretKey(bbsSecretKey *x, const void *buf, mclSize bufSize)
{
	if (bufSize < FR_SIZE) return 0;
	Fr& v = *cast(&x->v);
	if (v.deserialize(buf, FR_SIZE) != FR_SIZE || v.isZero()) return 0;
	return FR_SIZE;
}

// octets_to_pubkey of the spec
mclSize bbsDeserializePublicKey(bbsPublicKey *x, const void *buf, mclSize bufSize)
{
	if (bufSize < G2_SIZE) return 0;
	G2& v = *cast(&x->v);
	if (v.deserialize(buf, G2_SIZE) != G2_SIZE || v.isZero()) return 0;
	return G2_SIZE;
}

// octets_to_signature of the spec
mclSize bbsDeserializeSignature(bbsSignature *x, const void *buf, mclSize bufSize)
{
	if (bufSize < G1_SIZE + FR_SIZE) return 0;
	const uint8_t *p = (const uint8_t*)buf;
	if (!getG1(*cast(&x->A), p)) return 0;
	if (!getFr(*cast(&x->e), p + G1_SIZE)) return 0;
	return G1_SIZE + FR_SIZE;
}

mclSize bbsSerializeSecretKey(void *buf, mclSize maxBufSize, const bbsSecretKey *x)
{
	return cast(&x->v)->serialize(buf, maxBufSize);
}

mclSize bbsSerializePublicKey(void *buf, mclSize maxBufSize, const bbsPublicKey *x)
{
	return cast(&x->v)->serialize(buf, maxBufSize);
}

// signature_to_octets of the spec
mclSize bbsSerializeSignature(void *buf, mclSize maxBufSize, const bbsSignature *x)
{
	if (maxBufSize < G1_SIZE + FR_SIZE) return 0;
	uint8_t *p = (uint8_t*)buf;
	if (cast(&x->A)->serialize(p, G1_SIZE) != G1_SIZE) return 0;
	if (cast(&x->e)->serialize(p + G1_SIZE, FR_SIZE) != FR_SIZE) return 0;
	return G1_SIZE + FR_SIZE;
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
	return *cast(&lhs->A) == *cast(&rhs->A) && *cast(&lhs->e) == *cast(&rhs->e);
}

bool bbsInit(int cipherSuite, uint32_t maxMsgN)
{
	return bbs::init(cipherSuite, maxMsgN);
}

void bbsTerm()
{
	bbs::term();
}

bool bbsKeyGen(bbsSecretKey *sec, const uint8_t *keyMaterial, mclSize keyMaterialSize, const uint8_t *keyInfo, mclSize keyInfoSize, const uint8_t *keyDst, mclSize keyDstSize)
{
	return cast(sec)->keyGen(keyMaterial, keyMaterialSize, keyInfo, keyInfoSize, keyDst, keyDstSize);
}

bool bbsInitSecretKey(bbsSecretKey *sec)
{
	return cast(sec)->init();
}

bool bbsGetPublicKey(bbsPublicKey *pub, const bbsSecretKey *sec)
{
	if (!isInitialized()) return false;
	cast(sec)->getPublicKey(*cast(pub));
	return true;
}

bool bbsSign(bbsSignature *sig, const bbsSecretKey *sec, const bbsPublicKey *pub, const uint8_t *header, mclSize headerSize, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN)
{
	return cast(sig)->sign(*cast(sec), *cast(pub), header, headerSize, msgs, msgSize, msgN);
}

bool bbsVerify(const bbsSignature *sig, const bbsPublicKey *pub, const uint8_t *header, mclSize headerSize, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN)
{
	return cast(sig)->verify(*cast(pub), header, headerSize, msgs, msgSize, msgN);
}

mclSize bbsProofGen(uint8_t *proof, mclSize maxProofSize, const bbsPublicKey *pub, const bbsSignature *sig, const uint8_t *header, mclSize headerSize, const uint8_t *ph, mclSize phSize, const uint8_t *msgs, const uint32_t *msgSize, uint32_t msgN, const uint32_t *discIdxs, uint32_t discN)
{
	return bbs::proofGen(proof, maxProofSize, *cast(pub), *cast(sig), header, headerSize, ph, phSize, msgs, msgSize, msgN, discIdxs, discN);
}

bool bbsProofVerify(const bbsPublicKey *pub, const uint8_t *proof, mclSize proofSize, const uint8_t *header, mclSize headerSize, const uint8_t *ph, mclSize phSize, const uint8_t *discMsgs, const uint32_t *discMsgSize, const uint32_t *discIdxs, uint32_t discN)
{
	return bbs::proofVerify(*cast(pub), proof, proofSize, header, headerSize, ph, phSize, discMsgs, discMsgSize, discIdxs, discN);
}
