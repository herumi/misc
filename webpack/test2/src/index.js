exports.init = function() {
    return new Promise(resolve => {
//      const wasmCode = Buffer.from(data.wasmBase64, 'base64')
      const js = require(`./add.js`)
      const Module = {
//        wasmBinary: wasmBinary,
      }
      js(Module)
        .then(_mod => {
          exports.mod = _mod
          resolve()
        })
    })
}
