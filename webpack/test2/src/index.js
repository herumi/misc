const createModule = require(`./add.js`)
exports.init = async() => {
  console.log('exports.init')
  exports.mod = await createModule()
}
