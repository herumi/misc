import sys
sys.path.append('../')
from s_xbyak import *
import argparse

def maskTest():
  print(xmm1|k2)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('-win', '--win', help='output win64 abi', action='store_true')
  parser.add_argument('-n', '--num', help='max size of Unit', type=int, default=9)
  parser.add_argument('-addn', '--addn', help='max size of add/sub', type=int, default=16)
  parser.add_argument('-gas', '--gas', help='output gas syntax', default=False, action='store_true')
  parser.add_argument('-m', '--mode', help='output asm syntax', default='nasm')
  global param
  param = parser.parse_args()

  setWin64ABI(param.win)
  N = param.num
  addN = param.addn

  init(param.mode)
  segment('text')

  maskTest()

  term()

if __name__ == '__main__':
  main()
