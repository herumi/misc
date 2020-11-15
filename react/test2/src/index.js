import React from "react";
import ReactDOM from "react-dom";
import mcl from "mcl-wasm/browser/src/index-browser.js";

console.log('AAA')

class Layout extends React.Component {
  constructor () {
    super()
    this.state = { msg:"initializing..." }
    console.log('BBB')
    console.log(mcl)
    mcl.init().then(() => {
      const x = new mcl.Fr()
      x.setStr('123456')
      this.setState({
        msg: x.getStr()
      })
    })
  }
  render () {
    return <h2>{this.state.msg}</h2>
  }
}

const app = document.getElementById('app');
ReactDOM.render(<Layout/>, app);

