(gen => {
  gen(exports)
})(exports => {
  const fs = require('fs')
  const buf = fs.readFileSync('./addsub.wasm')
  // 1 = 64KiB
  const memory = new WebAssembly.Memory({initial:1, maximum:1})
  console.log(`memory=${memory}`)
  const imports = {
    env : {
      mulJS : (x, y) => { return x * y },
      memory: memory,
    }
  }
  WebAssembly.instantiate(buf, imports).then(
    ret => {
      exports.ret = ret
      exports.mod = ret.instance.exports
      const mod = exports.mod
      const mem = exports.mod.memory.buffer
      console.log(`mem=${mem}`)
      exports.mem = mem
      const u32 = new Uint32Array(mem)
      const N = 8
      const INT_SIZE = N * 4
      const xPos = 0
      const yPos = INT_SIZE
      const zPos = INT_SIZE * 2
      for (let i = 0; i < N; i++) {
        u32[i] = i + 1
        u32[N + i] = i + 10
      }
      console.log('before')
      for (let i = 0; i < N * 3; i++) {
        console.log(`mem[${i}]=${u32[i]}`)
      }
      mod.add256(zPos, yPos, xPos)
      console.log('after')
      for (let i = 0; i < N * 3; i++) {
        console.log(`mem[${i}]=${u32[i]}`)
      }
    }
  )
  return exports
})
