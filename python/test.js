const HEADERS = {
  "Content-Type": "application/json; charset=utf-8",
}
const URL = 'http://localhost:8000/'

const getValue = name => { return document.getElementsByName(name)[0].value }
const setText = (name, val) => { document.getElementsByName(name)[0].innerText = val }

const printTable = (js) => {
  console.log(`json=${JSON.stringify(js)}`)
  setText('result', JSON.stringify(js))
}

const onAdd = () => {
  const key = getValue('key')
  const value = getValue('value')
  console.log(`ADD key=${key}, value=${value}`)
  const js = {}
  js[key] = value
  fetch(URL + 'add', {
    method: 'POST',
    headers: HEADERS,
    body: JSON.stringify(js)
  }).then(res => res.json())
  .then(js => {
    printTable(js)
  })
  .catch(err => console.error(err))
}

const onDel = () => {
  const key = getValue('key')
  const js = {}
  js[key] = ''
  fetch(URL + 'del', {
    method: 'POST',
    headers: HEADERS,
    body: JSON.stringify(js)
  }).then(res => res.json())
  .then(js => {
    printTable(js)
  })
  .catch(err => console.error(err))
}
