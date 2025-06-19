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

  const sec = new bbs.SecretKey()
  sec.init()
  console.log(`sec=${sec.serializeToHexStr()}`)
  const pub = sec.getPublicKey()
  console.log(`pub=${pub.serializeToHexStr()}`)
}
