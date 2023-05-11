import re
DASH='\u2014'
reCMOV=re.compile(r'CMOV[A-Z]*$')
XMM=re.compile(r'[xyz]mm')

FILE='m.txt'

text = open(FILE).read().split('\n')

excludeTbl = [
  'register',
  'Implementation',
  'from',
  'to',
  'void',
  'int',
]

def match(v):
  if len(v) >= 5: return False
  if reCMOV.match(v[0]): return True
  if v[0] not in [op, 'V' + op]: return False
  if len(v) > 1 and v[1][0] in ['(', '_']: return False
  if len(v) > 1 and v[1] in excludeTbl: return False
  return True

op='@@'    
for i in range(len(text)):
  line = text[i]
  v = line.split(DASH)
  if len(v) == 2:
    if v[0].isupper() or v[0] == 'CMOVcc':
      op = v[0]
  v = line.split(' ')
  if match(v):
    s = line
    if s[-1] == ',':
      s += text[i+1]
    if XMM.search(s):
      s = s.strip('*')
      print(s.lower())
  

