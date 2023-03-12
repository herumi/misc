Int = 1
Imm = 2
IntPtr = 3

eq = 1
neq = 2
ugt = 3
uge = 4
ult = 5
ule = 6
sgt = 7
sge = 8
slt = 9
sle = 10

g_text = []
g_undefLabel = {}
g_defLabelN = 1
g_undefLabelN = 1
g_globalIdx = 0

def output(s):
  g_text.append(s)

def getLine():
  return len(g_text)

def init():
  global g_text
  g_text = []

class Label:
  def __init__(self):
    self.n = 0
  def __str__(self):
    if self.n > 0:
      return getDefLabel(self.n)
    global g_undefLabel
    global g_undefLabelN
    if -self.n in g_undefLabel:
      g_undefLabel[-self.n].append(getLine())
    else:
      self.n = -g_undefLabelN
      g_undefLabelN += 1
      g_undefLabel.setdefault(-self.n, []).append(getLine())
    return getUndefLabel(-self.n)

def L(label):
  if type(label) != Label:
    raise Exception(f'bad type {label}')
  if label.n > 0:
    raise Exception(f'already defined {label}')
  lines = []
  if label.n < 0:
    global g_undefLabelN
    n = -label.n
    if n in g_undefLabel:
      lines = g_undefLabel[n]
      oldStr = getUndefLabel(n)
      del g_undefLabel[n]
  global g_defLabelN
  label.n = g_defLabelN
  g_defLabelN += 1
  if lines:
    newStr = getDefLabel(label.n)
    global g_text
    for line in lines:
      g_text[line] = g_text[line].replace(oldStr, newStr)
  output(f'{getDefLabel(label.n)}:')

class Function:
  def __init__(self, name, ret, *args, private=False):
    s = 'define '
    if private:
      s += 'private '
    s += f'{ret.getType()} @{name}('
    for i in range(len(args)):
      if i > 0:
        s += ', '
      s += args[i].getFullName()
    s += ')'
    output(s)
    output('{')

  def close(self):
    output('}')

  def __enter__(self):
    return self

  def __exit__(self, ex_type, ex_value, trace):
    self.close()

def genFunc(name):
  def f(*args):
    if not args:
      return output(name)
    s = ''
    for arg in args:
      if s != '':
        s += ', '
      if g_gas:
        if type(arg) == int:
          s += str(arg)
        else:
          s += str(arg)
      else:
        s += str(arg)
    return output(name + ' ' + s)
  return f

def CondTypeToStr(t):
  tbl = [
    "eq", "neq", "ugt", "uge", "ult", "ule", "sgt", "sge", "slt", "sle"
  ]
  return tbl[t-1]

def getGlobalIdx():
  global g_globalIdx
  g_globalIdx += 1
  return g_globalIdx

def resetGlobalIdx():
  global g_globalIdx
  g_globalIdx = 0

class Operand:
  def __init__(self, t, bit):
    self.t = t
    self.bit = bit
    self.imm = 0
    self.idx = getGlobalIdx()

  def getFullName(self, isAlias=True):
    if self.t == Int or self.t == IntPtr:
      return f'{self.getType()} {self.getName()}'
    raise Exception('no fullName')

  def getType(self):
    if self.t == Int:
      return f'i{self.bit}'
    if self.t == IntPtr:
      return f'i{self.bit}*'
    if self.t == Void:
      return 'void'
    raise Exception('no type')
   
  def getName(self):
    if self.t == Int or self.t == IntPtr:
      return f'%r{self.idx}'
    if self.t == Imm:
      return str(self.imm)
    raise Exception('no name')

def term():
  n = len(g_text)
  i = 0
  while i < n:
    s = g_text[i]
    print(s)
    i += 1

def opRXX(name, r, x1, x2):
  output(f'{r.getName()} = {name} i{x1.bit} {x1.getName()}, {x2.getName()}')

def opX(name, x1):
  output(f'{name} i{x1.bit} {x1.getName()}')

def add(x, y):
  r = Operand(Int, x.bit)
  opRXX('add', r, x, y)
  return r

def ret(x):
  opX('ret', x)
