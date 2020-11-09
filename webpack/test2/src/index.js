const createModule = require(`./add_c.js`)
const setupFactory = require(`./add.js`)

console.log('index.js')
module.exports = setupFactory(createModule)
