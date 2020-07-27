const mcl = require('../../mcl-wasm/mcl.js')
/*
	sample of OpenVote
*/

function computeY(pubs, i) {
  let Y = new mcl.G1()
  for (let j = 0; j < i; j++) {
    Y = mcl.add(Y, pubs[j])
  }
  for (let j = i + 1; j < pubs.length; j++) {
    Y = mcl.sub(Y, pubs[j])
  }
  return Y
}

function solveDLP(P, X) {
  const M = 100
  let Q = new mcl.G1()
  for (let i = 0; i < M; i++) {
    if (X.isEqual(Q)) return i;
    Q = mcl.add(Q, P)
  }
  console.log("can't solve DLP")
  return -1
}

mcl.init().then(() => {
  let votes = []
  if (process.argv.length == 2) {
    console.log('specify bit seq')
    return
  }
  for (let i = 2; i < process.argv.length; i++) {
    votes.push(process.argv[i] == '1' ? 1 : 0)
  }
  const n = votes.length
  console.log(`n=${n}`)
  const P = new mcl.G1()
  P.setHashOf('a')
  let secs = []
  let pubs = []
  let Ys = []
  let Zs = []
 
  // write public key 
  for (let i = 0; i < n; i++) {
    const s = new mcl.Fr()
    s.setByCSPRNG()
    secs.push(s)
    pubs.push(mcl.mul(P, s))
    console.log(`i=${i} votes=${votes[i]} p=${pubs[i].serializeToHexStr()}`)
  }
  // write Y to BBS
  for (let i = 0; i < n; i++) {
    Ys.push(computeY(pubs, i))
    console.log(`i=${i} Y=${Ys[i].serializeToHexStr()}`)
  }
  // compute Z
  for (let i = 0; i < n; i++) {
    let Z = mcl.mul(Ys[i], secs[i])
    if (votes[i]) {
      Z = mcl.add(Z, P)
    }
    Zs.push(Z)
    console.log(`i=${i} Z=${Zs[i].serializeToHexStr()}`)
  }
  // sum of Z
  let sum = Zs[0]
  for (let i = 1; i < n; i++) {
    sum = mcl.add(sum, Zs[i])
  }
  // solve DLP
  let ret = solveDLP(P, sum)
  console.log(ret)
//  console.log('ret=${ret}`)
})

