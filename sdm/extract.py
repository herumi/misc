import re
DASH='\u2014'
reCMOV=re.compile(r'CMOV[A-Z]*$')
XMM=re.compile(r'[xyz]mm')

FILE='325383-sdm-vol-2abcd.txt'
#FILE='aaa.txt'

ARGS_TBL=['<XMM0-2>', '<XMM0-6>', '<XMM0>',
  'AL', 'CL', 'AX', 'DX', 'EAX', 'ECX', 'EDX', 'RAX', 'RDX',
  'ST(i)',
  'imm',
  'k1', 'k2',
  'm8', 'm16', 'm32', 'm64', 'm128', 'm256', 'm512',
  'mm',
  'r8', 'r16', 'r32', 'r64',
  'r/m32', 'r/m64',
  'xmm', 'ymm', 'zmm',
  'vm32', 'vm64',
  '{k1}', '{sae}', '{er}',
  '{k2}',
  '.m256', # miss of SDM (VPAND ymm1, ymm2, ymm3/.m256)
]

BAD_ARGS_TBL=[
  'immediately', 'from', 'to', 'sign',
]

def validArg(s):
  for t in BAD_ARGS_TBL:
    if s.startswith(t):
      return False
  for t in ARGS_TBL:
    if s.startswith(t):
      return True
  return False

def matchArg(s):
  if s in ['0', '1', '2', '3', '4', '5', '6', '7']:
    return True
  v = s.split(' ')
  for e in v:
    if not validArg(e.strip()):
      return False
  return True

def parseArgs(args):
  r = []
  for p in args.split(','):
    p = p.strip().strip('*')
    if matchArg(p):
      r.append(p)
    else:
      return []
  return r

ALNUM = re.compile('[A-Z][A-Z0-9]+$')
def matchOp(op):
  if not ALNUM.match(op):
    return False
  if op in ['XMM', 'YMM', 'ZMM', 'IF', 'THEN', 'INPUT', 'S1']:
    return False
  return True

text = open(FILE).read().split('\n')

for i in range(len(text)):
  line = text[i]
  if line == '':
    continue
  op = line.split(' ')[0]
  if not matchOp(op):
    continue
  v = line[len(op)+1:]
  if v == '':
    continue
  if v[-1] == ',':
    v += text[i+1]
  args = parseArgs(v)
  if args == []:
    continue
#  for v in args:
#    print(v)
  s = op + ' ' + ','.join(args)
  print(s.lower())

