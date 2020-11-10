module.exports = {
//  mode: 'development',
  entry: './src/index.js',
  output: {
    path: __dirname + '/',
    library: 'bls',
    libraryTarget: 'umd',
    filename: 'index.js'
  },
  resolve: {
    fallback: {
      "path": false,
      "fs": false
    }
  },
//  target: "node"
  target: "web"
}
