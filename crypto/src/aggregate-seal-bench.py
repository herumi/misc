# make a table for Markdown from the resutl of SEAL benchmark.
# cd SEAL
# mkdir build
# cd build
# cmake .. -DSEAL_BUILD_EXAMPLES=ON -DSEAL_BUILD_BENCH=ON
# bin/sealbench > bench.txt
# python3 aggregate-seal-bench.py bench.txt > bench.md

import sys
import re
import pprint
from collections import defaultdict

if len(sys.argv) == 1:
  FILE='bench.txt'
else:
  FILE=sys.argv[1]

text = open(FILE).read().split('\n')

def removeBlank(v):
  while '' in v:
    v.remove('')


tbl={}
for line in text:
  v = line.split(' ')
  removeBlank(v)
  if len(v) == 12:
    n=int(v[0][2:])
    log_q=int(v[2][7:])
    method=v[4]
    op=v[6].split('/')[0]
    t=v[7]
    tbl.setdefault(n,{})
    tbl[n].setdefault(log_q,{})
    tbl[n][log_q].setdefault(op,{})
    tbl[n][log_q][op][method] = t

print('n|log(q)|method|BFV|BGV|CKKS|KeyGen')
print('-'+'|-'*6)
for (n, v) in sorted(tbl.items()):
  for (log_q, v1) in sorted(v.items()):
    if log_q > 0:
      for (method, v2) in sorted(v1.items()):
        tBFV = v2.get('BFV', '')
        tBGV = v2.get('BGV', '')
        tCKKS = v2.get('CKKS', '')
        tKeyGen = v2.get('KeyGen', '')
        if tBFV != '' or tBGV != '' or tCKKS  != '' or tKeyGen != '':
          print(f'{n}|{log_q}|{method}|{tBFV}|{tBGV}|{tCKKS}|{tKeyGen}')
   
  
