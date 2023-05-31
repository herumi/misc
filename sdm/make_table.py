import re
import pprint
from collections import defaultdict

FILE='avx.txt'

text = open(FILE).read().split('\n')

# op reg, [m]
RegMemTbl = {}
# op [m], reg, ...
MemRegTbl = {}

#RE_ARGS = re.compile(r'(xmm|ymm|zmm|m128|m256|m512|m32bcst|m64bcst|m64|m32|imm)')
RE_MEM = re.compile(r'(m32|m64|m128|m256|m512)')
RE_XMM = re.compile(r'(xmm|ymm|zmm)')
X = 'xmm'
Y = 'ymm'
Z = 'zmm'
M64 = 'm64'
MX = 'm128'
MY = 'm256'
MZ = 'm512'
M32b = 'm32bcst'
M64b = 'm64bcst'

def parse(arg):
  m = RE_MEM.search(arg)
  if m:
    return m.group(1)
  m = RE_XMM.search(arg)
  if m:
    return m.group(1)
  arg = arg.strip()
  if arg == 'imm8' or '01234567'.find(arg) >= 0:
    return 'imm'
  if arg.startswith('k1'):
    return 'k'
  return None

for line in text:
  if line == '':
    break
  op = line.split(' ')[0]
  v = line[len(op):].split(',')
  args = []
  for p in v:
    x = parse(p)
    if not x:
      break
    args.append(x)
  if len(args) < len(v):
    continue

  tbl = None
  if args[0][0] == 'm':
    tbl = MemRegTbl
  else:
    tbl = RegMemTbl
  tbl.setdefault(op, set()).add(tuple(args))

print('MemRegTbl=',end='')
pprint.pprint(MemRegTbl)
print('RegMemTbl=',end='')
pprint.pprint(RegMemTbl)
