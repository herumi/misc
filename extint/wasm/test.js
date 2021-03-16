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
  const { performance } = require('perf_hooks')
  const bench = (label, count, func) => {
    const start = performance.now()
    for (let i = 0; i < count; i++) {
      func()
    }
    const end = performance.now()
    const t = (end - start) / count
    const roundTime = (Math.round(t * 1e9)) / 1000
    console.log(label + ' ' + roundTime + ' nsec')
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
      const BYTE_SIZE = N * 4
      const xPos = 0
      const yPos = BYTE_SIZE
      const z1Pos = BYTE_SIZE * 2
      const z2Pos = BYTE_SIZE * 3
      const z3Pos = BYTE_SIZE * 4
      const dump = (msg, pos) => {
        console.log(`msg=${msg}, pos=${pos}`)
        for (let i = 0; i < N; i += 4) {
          console.log(`${u32[pos/4+i]} ${u32[pos/4+i+1]} ${u32[pos/4+i+2]} ${u32[pos/4+i+3]}`)
        }
      }
      const cmpArray = (pos1, pos2) => {
        for (let i = 0; i < N; i++) {
          const v1 = u32[pos1/4+i]
          const v2 = u32[pos2/4+i]
          if (v1 !== v2) {
            console.log(`QQQ i=${i} v1=${v1} v2=${v2}`)
          }
        }
      }
      for (let i = 0; i < N; i++) {
        u32[i] = i * 2147483643 + 1
        u32[N + i] = i * 2147483642 + 11
      }
      dump('x', xPos)
      dump('y', yPos)
      console.log('before')
      mod.add256_extInt(z1Pos, yPos, xPos)
      mod.add256_u32(z2Pos, yPos, xPos)
      mod.add256_u64(z3Pos, yPos, xPos)
      console.log('after')
      cmpArray(z1Pos, z2Pos)
      cmpArray(z1Pos, z3Pos)
      dump('z', z1Pos)
      const C = 100000000
      bench('extInt', C, () => { mod.add256_extInt(yPos, yPos, yPos) })
    }
  )
  return exports
})
