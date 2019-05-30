"use strict"

async function initWasm() {
  const fs = require('fs')
  const wasm = fs.readFileSync('./add.wasm')
  const memory = await new WebAssembly.Memory({initial:100})
  const imports = {
    env : {
      memory: memory,
    }
  }
  const ret = await WebAssembly.instantiate(wasm, imports)
  return ret.instance.exports
}

let mie = {};

(async () => {
  console.log('async')
  mie.func = await initWasm()
  await test()
//  mie.mem = await new Memory(mie.func.memory.buffer)
})()

const test = () => {
  console.log('test')
  const ret = mie.func.add(3, 5)
  console.log(`ret=${ret}`)
}
