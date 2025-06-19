import * as bbs from '../dist'
import * as assert from 'assert'

async function testAll () {
  try {
    console.log('Initializing BBS')
    await bbs.init()
    bbsTest()
    proofTest()
  } catch (e) {
    console.log(`TEST FAIL ${e}`)
    console.log('Error stack:', e.stack)
    assert(false)
  }
}

testAll()

const bbsTest = () => {
  console.log('BBS signature')

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
      const sig = bbs.sign(sec, pub, msgs)
      {
        const s = sig.serializeToHexStr()
        const sig2 = bbs.deserializeHexStrToSignature(s)
        const s2 = sig2.serializeToHexStr()
        console.log(`s =${s}, len=${s.length}`)
        console.log(`s2=${s2}, len=${s2.length}`)
        console.log('eq =', sig.isEqual(sig2), s === s2, sig.isEqual(sig), sig2.isEqual(sig2))
        assert(bbs.verify(sig2, pub, msgs))
        // assert(sig.isEqual(sig2))
        console.log(`sig=${s}`)
      }

      assert(bbs.verify(sig, pub, msgs))
      msgs[0][0] += 1
      assert(!bbs.verify(sig, pub, msgs))
    } catch (e) {
      console.log(`Error in iteration ${i}:`, e)
      console.log('Error stack:', e.stack)
      throw e
    }
  }
}

// return msgs[discIdxs[i]]
const getDiscMsgs = (msgs: Uint8Array[], discIdxs: Uint32Array): Uint8Array[] => {
  const r = []
  for (let i = 0; i < discIdxs.length; i++) {
    r.push(msgs[discIdxs[i]])
  }
  return r
}

const proofTest = () => {
  console.log('Proof')
  const sec = new bbs.SecretKey()
  sec.init()
  const pub = sec.getPublicKey()
  const msgs = [new Uint8Array([1, 2, 3]), new Uint8Array([4, 5, 6, 7, 8, 9]), new Uint8Array([10, 11, 12, 13])]
  const sig = bbs.sign(sec, pub, msgs)
  const discIdxs = new Uint32Array([0, 2])
  const discMsgs = getDiscMsgs(msgs, discIdxs)
  const nonce = new Uint8Array([1, 2, 3])
  const prf = bbs.createProof(pub, sig, discMsgs, discIdxs, nonce)

  assert(bbs.verifyProof(pub, prf, discMsgs, discIdxs, nonce))
  bbs.destroyProof(prf)
}
