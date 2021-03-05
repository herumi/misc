(gen => {
  console.log(`exports=${exports}`)
  gen(exports)
})(exports => {
  console.log(`exports 2=${exports}`)
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
      const mem = exports.mod.memory.buffer
      exports.mem = mem
      const u8 = new Uint8Array(mem)
      console.log(`add=${exports.mod.add(12, 34)}`)
      console.log(`sub=${exports.mod.sub(12, 34)}`)
      console.log(`callJS=${exports.mod.callJS(3, 5)}`)
      exports.mod.setMem(u8, 10)
      for (let i = 0; i < 10; i++) {
        console.log(`mem[${i}]=${u8[i]}`)
      }
      console.log(`mem length=${mem.byteLength}`)
      console.log(`getPtr=${exports.mod.getPtr(4)}`)
      console.log(`getPtr=${exports.mod.getPtr(32)}`)
      console.log(`getPtr=${exports.mod.getPtr(64)}`)
    }
  )
  return exports
})
