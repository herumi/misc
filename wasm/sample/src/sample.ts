const createModule = require('./sample_c.js')

export const initializeSample = async (x:number) => {
  console.log('initializeSample')
  const mod = await createModule()
  const r = mod._func(x)
  console.log(`func(${x})=${r}`)
}
