const ex = require('./index.js')

ex.init().then(() => {
  try {
    console.log('nocatch')
    const f = ex.mod.func_nocatch
    console.log(`nocatch 5 -> ${f(5)}`)
    console.log(`nocatch -5 -> ${f(-5)}`)
  } catch (e) {
    console.log(`err ${e}`)
  }
  try {
    console.log('catch')
    const g = ex.mod.func_catch
    console.log(`catch 5 -> ${g(5)}`)
    console.log(`catch -5 -> ${g(-5)}`)
  } catch (e) {
    console.log(`err ${e}`)
  }
})
