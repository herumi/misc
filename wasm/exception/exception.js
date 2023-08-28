const setupFactory = (createModule) => {
  const exports = {}

  const setup = (exports) => {
    const mod = exports.mod
    mod.func_nocatch = mod._func_nocatch
    mod.func_catch = mod._func_catch
  }

  exports.init = async () => {
    exports.mod = await createModule({
    })
    setup(exports)
  }
  return exports
}

module.exports = setupFactory

