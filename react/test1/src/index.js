import React from 'react';
import ReactDOM from 'react-dom';
import mcl from 'mcl-wasm';

class Layout extends React.Component {
  constructor() {
    super();
    this.state = { msg:"initializing..." }
    mcl.init().then(() => {
      const x = new mcl.Fr()
      x.setStr('123')
      this.setState({
        msg: x.getStr()
      })
    })
  }
  render() {
    return (
      <h2>{this.state.msg}</h2>
    );
  }
}

const app = document.getElementById('app');
ReactDOM.render(<Layout />, app);

