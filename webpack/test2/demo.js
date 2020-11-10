function setValue(name, val) { document.getElementsByName(name)[0].innerHTML = val }

function test() {
  console.log(bls)
  bls.init().then(() => {
    const a = bls.mod._add(3, 4)
    setValue('ret', a)
  })
}
