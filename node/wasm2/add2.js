"use strict"

console.log('AAA')
const mod = require("./add.js")
exports.mod = mod
let initCb = null

exports.onModuleInit = function(cb) {
	console.log('onModuleInit')
	initCb = cb
}
mod.onRuntimeInitialized = () => {
	console.log('onRuntimeInitialized')
	initCb()
}

console.log('BBB')
