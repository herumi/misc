const lib = require('./index.js')

lib.init().then(() => {
  console.log('test.js')
  console.log(`add(3,5)=${lib.mod._add(3,5)}`)
})
