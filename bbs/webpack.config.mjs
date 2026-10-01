import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

export default {
  mode: 'production',
  entry: './wasm/bbs.ts',
  target: 'web',
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: {
          loader: 'ts-loader',
          options: {
            compilerOptions: {
              module: 'commonjs',
              target: 'es2020',
              lib: ['es2020', 'dom'],
              allowJs: true,
              esModuleInterop: true,
              moduleResolution: 'node'
            }
          }
        },
        exclude: /node_modules/,
      },
    ],
  },
  resolve: {
    extensions: ['.tsx', '.ts', '.js'],
  },
  output: {
    filename: 'bbs.js',
    path: path.resolve(__dirname, 'browser'),
    library: 'bbs',
    libraryTarget: 'umd',
    globalObject: 'this',
  },
  optimization: {
    minimize: false,
  },
  externals: {
    'fs': 'commonjs fs',
    'path': 'commonjs path'
  }
} 