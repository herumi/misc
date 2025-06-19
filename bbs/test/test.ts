import * as bbs from '../dist'
import * as assert from 'assert'

async function testAll() {
  await bbs.init()
  try {
    console.log(`bbs.init`)
    bbsTest()
  } catch (e) {
    console.log(`TEST FAIL ${e}`)
    assert(false)
  }
}

testAll()

const bbsTest = () => {
  console.log('BBS signature')

  for (let i = 0; i < 3; i++) {
    console.log(`i=${i}`)
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
      const pub2 = bbs.deserializeHexStrToPublicKey   (s)
      assert(pub.isEqual(pub2))
      console.log(`pub=${s}`)
    }
  }
}
