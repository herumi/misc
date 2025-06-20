import * as bbs from '../dist'
import * as assert from 'assert'

async function testAll () {
  try {
    console.log('Initializing BBS')
    await bbs.init()
    bbsTest()
    fixedTest()
    proofTest()
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
      const sig = bbs.sign(sec, pub, msgs)
      let sig2 = new bbs.Signature()
      {
        const s = sig.serializeToHexStr()
        sig2 = bbs.deserializeHexStrToSignature(s)
        assert(sig.isEqual(sig2))
        console.log(`sig=${s}`)
      }

      assert(bbs.verify(sig, pub, msgs))
      assert(bbs.verify(sig2, pub, msgs))
      msgs[0][0] += 1
      assert(!bbs.verify(sig, pub, msgs))
      assert(!bbs.verify(sig2, pub, msgs))
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
  const sig = bbs.sign(sec, pub, msgs)
  const discIdxs = new Uint32Array([0, 2])
  const discMsgs = getDiscMsgs(msgs, discIdxs)
  console.log('discMsgs=', discMsgs)
  const nonce = new Uint8Array([1, 2, 3])
  const prf = bbs.createProof(pub, sig, msgs, discIdxs, nonce)
  assert(prf.pos !== 0)

  assert(bbs.verifyProof(pub, prf, discMsgs, discIdxs, nonce))
  bbs.destroyProof(prf)
}

// generate Uint8Array from ascii string
const strToUint8Array = (s: string): Uint8Array => {
  return new Uint8Array(s.split('').map(c => c.charCodeAt(0)))
}

const fixedTest = () => {
  console.log('Fixed test')
  const secHex = '6528255759bb6c2c64fed04877398200f67642bddb8bfe200690db6a30487811'
//  const secHex = '0000000000000000000000000000000000000000000000000000000000000001'
  const sec = bbs.deserializeHexStrToSecretKey(secHex)
  console.log('sec=', sec.serializeToHexStr())
  const pub = sec.getPublicKey()
  console.log('pub=', pub.serializeToHexStr())

  const msgTbl = ['v', 'kbv', 'qnmnq', 'vbkvhwm', 'ez', 'vttv', 'zemwhv', 'k', 'bvq', 'nmnqv']
  const msgs: Uint8Array[] = msgTbl.map(strToUint8Array)
  const sig = bbs.sign(sec, pub, msgs)
  console.log('sig=', sig.serializeToHexStr())

  const discIdxs = new Uint32Array([1, 4, 5])
  const nonce = new Uint8Array([9, 0x11, 0x22])
  const prf = bbs.createProof(pub, sig, msgs, discIdxs, nonce)
  assert(prf.pos !== 0)
  console.log('prf=', prf.serializeToHexStr())
  if (false){
    console.log('------------wasm-----------------')
    const s2 = sig.serializeToHexStr()
    console.log('sig.A=', s2.substring(0, 96))
    console.log('sig.e=', s2.substring(96, 160))
    console.log('sig.s=', s2.substring(160, 224))
    const s = prf.serializeToHexStr()
    console.log('prf.A_prime=', s.substring(0, 96))
    console.log('prf.A_bar=', s.substring(96, 192))
    console.log('prf.D=', s.substring(192, 288))
    console.log('prf.c=', s.substring(288, 352))
//    e_hat, r2_hat, r3_hat, s_hat
    console.log('prf.e_hat=', s.substring(352, 416))
    console.log('prf.r2_hat=', s.substring(416, 480))
    console.log('prf.r3_hat=', s.substring(480, 544))
    console.log('prf.s_hat=', s.substring(544, 608))
    console.log('--------------------------------')
  }
  const discMsgs = getDiscMsgs(msgs, discIdxs)
  assert(bbs.verifyProof(pub, prf, discMsgs, discIdxs, nonce))
  bbs.destroyProof(prf)
}
