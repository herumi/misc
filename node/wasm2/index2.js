const m = require('./add2.js')

console.log('111')
m.onModuleInit(function() {
	console.log('222')
	console.log(m.mod._add(3, 5))
})
console.log('333')

