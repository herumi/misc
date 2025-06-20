
module.exports = {
  mode: 'production',
  entry: './wasm/index.ts',
  output: {
    path: __dirname + '/browser/',
    library: 'bbs',
    libraryTarget: 'umd',
    filename: 'bbs.js'
  },
  module: {
    rules: [
      {
        test: /\.ts$/,
        use: 'ts-loader',
        exclude: /node_modules/,
      },
    ],
  },
  resolve: {
    extensions: ['.ts', '.js'],
    fallback: {
      path: false,
      fs: false,
      crypto: false
    }
  },
  target: 'web'
}
