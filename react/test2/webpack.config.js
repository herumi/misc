const path  = require('path');

module.exports = {
  mode: 'development',
  entry: './src/index.js',
/*
  devServer: {
    host: '0.0.0.0',
    port: 3000
  },
*/
  output: {
    filename: 'main.js',
    path: path.resolve(__dirname, 'public')
  },
  resolve: {
    fallback: {
      path: false,
      fs: false,
      crypto: false
    },
  },
  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        loader: 'babel-loader'
      }
    ]
  }
};

