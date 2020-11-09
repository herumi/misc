function setupFactory(createModule) {
  const exports = {}
  exports.init = async () => {
    console.log('exports.init')
    exports.mod = await createModule()
  }
  return exports
}

// NodeJS export
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = setupFactory
  console.log('module.exports')
}
