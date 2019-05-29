"use strict"

const add = require("./add.js")
add.onRuntimeInitialized = () => {
	console.log(add._add(10, 20))
}
