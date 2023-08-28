const createModule = require('./exception_c.js')
const setupFactory = require('./exception')

const ex = setupFactory(createModule)

module.exports = ex
