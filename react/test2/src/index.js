import React from "react";
import ReactDOM from "react-dom";
import * as mcl from './mcl-wasm';

class Layout extends React.Component {
  constructor () {
    super()
    this.state = { msg:"initializing..." }
    mcl.init().then(() => {
      const x = new mcl.Fr()
      x.setStr('12345678')
      const y = new mcl.Fr()
      y.setByCSPRNG()
      this.setState({
        msg: x.getStr() + ':' + y.getStr()
      })
    })
  }
  render () {
    return <h2>{this.state.msg}</h2>
  }
}

const app = document.getElementById('app');
ReactDOM.render(<Layout/>, app);

