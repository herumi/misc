const lib = require('./index.js')

lib.init().then(() => {
  console.log('QQQ')
  console.log(`add(3,5)=${lib.mod._add(3,5)}`)
})
