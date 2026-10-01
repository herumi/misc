import * as bbs from '../dist'
import * as assert from 'assert'

async function testAll () {
  try {
    console.log('Initializing BBS')
    await bbs.init()
    bbsTest()
    keyGenTest()
    fixedTest()
    proofTest()
    bbs.term()
  } catch (e) {
    console.log(`TEST FAIL ${e}`)
    console.log('Error stack:', e.stack)
    assert(false)
  }
}

testAll()

const bbsTest = () => {
  console.log('bbsTest')

  for (let i = 0; i < 3; i++) {
    console.log(`i=${i}`)
    try {
      const sec = new bbs.SecretKey()
      sec.init()
      {
        const s = sec.serializeToHexStr()
        const sec2 = bbs.deserializeHexStrToSecretKey(s)
        assert(sec.isEqual(sec2))
        console.log(`sec=${s}`)
      }
      const pub = sec.getPublicKey()
      {
        const s = pub.serializeToHexStr()
        const pub2 = bbs.deserializeHexStrToPublicKey(s)
        assert(pub.isEqual(pub2))
        console.log(`pub=${s}`)
      }
      const msgs = [new Uint8Array([1, 2, 3]), new Uint8Array([4, 5, 6, 7, 8, 9]), new Uint8Array([10, 11, 12, 13])]
      const header = new Uint8Array([0x11, 0x22])
      const sig = bbs.sign(sec, pub, msgs, header)
      let sig2 = new bbs.Signature()
      {
        const s = sig.serializeToHexStr()
        sig2 = bbs.deserializeHexStrToSignature(s)
        assert(sig.isEqual(sig2))
        console.log(`sig=${s}`)
      }

      assert(bbs.verify(sig, pub, msgs, header))
      assert(bbs.verify(sig2, pub, msgs, header))
      // wrong header
      assert(!bbs.verify(sig, pub, msgs))
      assert(!bbs.verify(sig, pub, msgs, new Uint8Array([0x11])))
      msgs[0][0] += 1
      assert(!bbs.verify(sig, pub, msgs, header))
      assert(!bbs.verify(sig2, pub, msgs, header))

      // header is optional
      const sig3 = bbs.sign(sec, pub, msgs)
      assert(bbs.verify(sig3, pub, msgs))
      assert(!bbs.verify(sig3, pub, msgs, header))
    } catch (e) {
      console.log(`Error in iteration ${i}:`, e)
      console.log('Error stack:', e.stack)
      throw e
    }
  }
}

// return msgs[discIdxs[i]]
const getDiscMsgs = (msgs: Uint8Array[], discIdxs: Uint32Array): Uint8Array[] => {
  const r: Uint8Array[] = []
  for (let i = 0; i < discIdxs.length; i++) {
    r.push(msgs[discIdxs[i]])
  }
  return r
}

// 8.4.1 Key Pair of draft-irtf-cfrg-bbs-signatures-12
const keyGenTest = () => {
  console.log('keyGenTest')
  const keyMaterial = bbs.fromHexStr('746869732d49532d6a7573742d616e2d546573742d494b4d2d746f2d67656e65726174652d246528724074232d6b6579')
  const keyInfo = bbs.fromHexStr('746869732d49532d736f6d652d6b65792d6d657461646174612d746f2d62652d757365642d696e2d746573742d6b65792d67656e')
  const keyDst = strToUint8Array('BBS_BLS12381G1_XMD:SHA-256_SSWU_RO_H2G_HM2S_KEYGEN_DST_')
  const sec = new bbs.SecretKey()
  sec.keyGen(keyMaterial, keyInfo, keyDst)
  assert.equal(sec.serializeToHexStr(), '60e55110f76883a13d030b2f6bd11883422d5abde717569fc0731f51237169fc')
  const pub = sec.getPublicKey()
  assert.equal(pub.serializeToHexStr(), 'a820f230f6ae38503b86c70dc50b61c58a77e45c39ab25c0652bbaa8fa136f2851bd4781c9dcde39fc9d1d52c9e60268061e7d7632171d91aa8d460acee0e96f1e7c4cfb12d3ff9ab5d5dc91c277db75c845d649ef3c4f63aebc364cd55ded0c')

  // 8.4.4.1 Valid Single Message Signature
  const msgs = [bbs.fromHexStr('9872ad089e452c7b6e283dfac2a80d58e8d0ff71cc4d5e310a1debdda4a45f02')]
  const header = bbs.fromHexStr('11223344556677889900aabbccddeeff')
  const sig = bbs.sign(sec, pub, msgs, header)
  assert.equal(sig.serializeToHexStr(), '84773160b824e194073a57493dac1a20b667af70cd2352d8af241c77658da5253aa8458317cca0eae615690d55b1f27164657dcafee1d5c1973947aa70e2cfbb4c892340be5969920d0916067b4565a0')
  assert(bbs.verify(sig, pub, msgs, header))

  // the default dst is used
  const sec2 = new bbs.SecretKey()
  sec2.keyGen(keyMaterial, keyInfo)
  assert(!sec.isEqual(sec2))
  // keyMaterial is too short
  assert.throws(() => { sec2.keyGen(new Uint8Array(31)) })
}

const proofTest = () => {
  console.log('ProofTest')
  const sec = new bbs.SecretKey()
  sec.init()
  const pub = sec.getPublicKey()
  const msgs: Uint8Array[] = [
    new Uint8Array([1, 2, 3]),
    new Uint8Array([4, 5, 6, 7, 8, 9]),
    new Uint8Array([10, 11, 12, 13])
  ]
  const header = new Uint8Array([0x11, 0x22])
  const sig = bbs.sign(sec, pub, msgs, header)
  const discIdxs = new Uint32Array([0, 2])
  const discMsgs = getDiscMsgs(msgs, discIdxs)
  console.log('discMsgs=', discMsgs)
  const ph = new Uint8Array([1, 2, 3])
  const prf = bbs.proofGen(pub, sig, msgs, discIdxs, header, ph)
  assert.equal(prf.length, bbs.getProofSize(msgs.length - discIdxs.length))
  assert.equal(prf.length, 48 * 3 + 32 * (4 + 1))

  assert(bbs.proofVerify(pub, prf, discMsgs, discIdxs, header, ph))
  // wrong presentation header
  assert(!bbs.proofVerify(pub, prf, discMsgs, discIdxs, header))
  assert(!bbs.proofVerify(pub, prf, discMsgs, discIdxs, header, new Uint8Array([1, 2])))
  // wrong header
  assert(!bbs.proofVerify(pub, prf, discMsgs, discIdxs, undefined, ph))
  // wrong message
  discMsgs[0] = new Uint8Array([1, 2, 4])
  assert(!bbs.proofVerify(pub, prf, discMsgs, discIdxs, header, ph))
  discMsgs[0] = msgs[0]
  // modified proof
  const prf2 = new Uint8Array(prf)
  prf2[prf2.length - 1] ^= 1
  assert(!bbs.proofVerify(pub, prf2, discMsgs, discIdxs, header, ph))
  assert(!bbs.proofVerify(pub, prf.subarray(0, prf.length - 1), discMsgs, discIdxs, header, ph))

  // proofs are randomized
  const prf3 = bbs.proofGen(pub, sig, msgs, discIdxs, header, ph)
  assert(bbs.toHexStr(prf) !== bbs.toHexStr(prf3))
  assert(bbs.proofVerify(pub, prf3, discMsgs, discIdxs, header, ph))

  // disclose nothing / all without header and ph
  const sig2 = bbs.sign(sec, pub, msgs)
  const none = new Uint32Array([])
  const prf4 = bbs.proofGen(pub, sig2, msgs, none)
  assert(bbs.proofVerify(pub, prf4, [], none))
  const all = new Uint32Array([0, 1, 2])
  const prf5 = bbs.proofGen(pub, sig2, msgs, all)
  assert(bbs.proofVerify(pub, prf5, msgs, all))

  // bad index
  assert.throws(() => { bbs.proofGen(pub, sig2, msgs, new Uint32Array([2, 0])) })
  assert.throws(() => { bbs.proofGen(pub, sig2, msgs, new Uint32Array([0, 3])) })
}

// generate Uint8Array from ascii string
const strToUint8Array = (s: string): Uint8Array => {
  return new Uint8Array(s.split('').map(c => c.charCodeAt(0)))
}

// the same values as fixed test in bbs_test.cpp
const fixedTest = () => {
  console.log('Fixed test')
  const secHex = '6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811'
  const sec = bbs.deserializeHexStrToSecretKey(secHex)
  console.log('sec=', sec.serializeToHexStr())
  const pub = sec.getPublicKey()
  console.log('pub=', pub.serializeToHexStr())
  assert.equal(pub.serializeToHexStr(), 'b06e2a39e47c4fc65cf1d51dd181b793a57ebc4a3dc35bb8245c804ecc9b39effd5516c260ba463bafc1a1e002da9cba17f35c0d1b4c0518779d134cbd1b967996cc3f3de4c8e9a20c8c1db8f759439f16c995a9d25e861cb4eee282d9a2d085')

  const msgTbl = ['v', 'kbv', 'qnmnq', 'vbkvhwm', 'ez', 'vttv', 'zemwhv', 'k', 'bvq', 'nmnqv']
  const msgs: Uint8Array[] = msgTbl.map(strToUint8Array)
  const header = strToUint8Array('header')
  const sig = bbs.sign(sec, pub, msgs, header)
  console.log('sig=', sig.serializeToHexStr())
  assert.equal(sig.serializeToHexStr(), '8db34eb67d85d70022d5875a02ea095a7035481d1bacbbf37d2e68c321b4e9165f2390bb688b642e7327fcc5ef59aae82b49108ed81da5e66818ab4e95c75dd5e896855fb2ab6e9104f7b6a05b804a50')

  assert(bbs.verify(sig, pub, msgs, header))

  const discIdxs = new Uint32Array([1, 4, 5])
  const ph = new Uint8Array([9, 0x11, 0x22])
  const prf = bbs.proofGen(pub, sig, msgs, discIdxs, header, ph)
  console.log('prf=', bbs.toHexStr(prf))
  const discMsgs = getDiscMsgs(msgs, discIdxs)
  assert(bbs.proofVerify(pub, prf, discMsgs, discIdxs, header, ph))
}
