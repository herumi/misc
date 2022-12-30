'use strict'
const sample = require('../dist/index.js')

sample.init(123)
  .then(() => {
    console.log(`sample.init`)
  })

