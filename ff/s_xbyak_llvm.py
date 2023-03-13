VOID_TYPE = 0
INT_TYPE = 1
IMM_TYPE = 2
INT_PTR_TYPE = 3

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
  def __init__(self, name, ret, *args, private=False, noalias=True):
    self.name = name
    self.ret = ret
    self.args = args
    self.private = private
    self.noalias = noalias
    s = 'define '
    if private:
      s += 'private '
    s += f'{ret.getType()} @{name}('
    for i in range(len(args)):
      if i > 0:
        s += ', '
      s += args[i].getFullName(noalias)
    s += ')'
    output(s)
    output('{')

  def getName(self):
    return f'{self.ret.getType()} @{self.name}'

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
  def __init__(self, t, bit, imm=0):
    self.t = t
    self.bit = bit
    self.imm = imm
    if t != IMM_TYPE:
      self.idx = getGlobalIdx()

  def getFullName(self, noalias=False):
    return f'{self.getType(noalias)} {self.getName()}'

  def getType(self, noalias=False):
    if self.t == INT_TYPE or self.t == IMM_TYPE:
      return f'i{self.bit}'
    if self.t == INT_PTR_TYPE:
      if noalias:
        return f'i{self.bit}* noalias'
      else:
        return f'i{self.bit}*'
    if self.t == VOID_TYPE:
      return 'void'
    raise Exception('no type')
   
  def getName(self):
    if self.t == INT_TYPE or self.t == INT_PTR_TYPE:
      return f'%r{self.idx}'
    if self.t == IMM_TYPE:
      return str(self.imm)
    return ''

class Int(Operand):
  def __init__(self, bit):
    self = Operand.__init__(self, INT_TYPE, bit)

class IntPtr(Operand):
  def __init__(self, bit):
    self = Operand.__init__(self, INT_PTR_TYPE, bit)

class Imm(Operand):
  def __init__(self, imm):
    bit = int(imm).bit_length()
    bit = ((bit + 31) // 32) * 32
    if bit == 0:
      bit = 32
    self = Operand.__init__(self, IMM_TYPE, bit, imm)

Void = Operand(VOID_TYPE, 0)

def term():
  n = len(g_text)
  i = 0
  while i < n:
    s = g_text[i]
    print(s)
    i += 1

# r = op x, v

def genOp_r_x_v(name):
  def f(x, v):
    if type(v) == int:
      v = Imm(v)
    r = Int(x.bit)
    output(f'{r.getName()} = {name} {x.getFullName()}, {v.getName()}')
    return r
  return f

tbl = ['lshr', 'ashr', 'shl', 'add', 'sub', 'mul', 'and_', 'or_']
for name in tbl:
  llvmName = name.strip('_')
  globals()[name] = genOp_r_x_v(llvmName)

def select(cond, x, y):
  r = Int(x.bit)
  output(f'{r.getName()} = select {cond.getFullName()}, {x.getFullName()}, {y.getFullName()}')
  return r

# r = op x to y
def zext(x, bit):
  r = Int(bit)
  output(f'{r.getName()} = zext {x.getFullName()} to {r.getType()}')
  return r

def trunc(x, bit):
  r = Int(bit)
  output(f'{r.getName()} = trunc {x.getFullName()} to {r.getType()}')
  return r

def bitcast(x, bit):
  r = IntPtr(bit)
  output(f'{r.getName()} = bitcast {x.getFullName()} to {r.getType()}')
  return r



# op x
def ret(x):
  output(f'ret {x.getFullName()}')

# op x, v
def store(x, v):
  output(f'store {x.getFullName()}, {v.getFullName()}')

# r = op x
def load(x):
  r = Int(x.bit)
  output(f'{r.getName()} = load {r.getType()}, {x.getFullName()}')
  return r

def getelementptr(x, v):
  if type(v) == int:
    v = Imm(v)
  r = IntPtr(x.bit)
  output(f'{r.getName()} = getelementptr i{x.bit}, {x.getFullName()}, {v.getFullName()}')
  return r

def call(func, *args):
  s = ''
  if func.ret.t != VOID_TYPE:
    r = Operand(func.ret.t, func.ret.bit)
    s = f'{r.getName()} = '
  s += f'call {func.getName()}('
  for i in range(len(args)):
    if i > 0:
      s += ', '
    s += args[i].getFullName()
  s += ')'
  output(s)
  if func.ret.t != VOID_TYPE:
    return r

####

def loadN(p, n, offset=0):
  if offset != 0:
    p = getelementptr(p, offset)
  v = load(p)
  unit = v.bit
  for i in range(1, n):
    v = zext(v, v.bit + unit)
    t = load(getelementptr(p, i))
    t = zext(t, v.bit)
    t = shl(t, unit * i)
    v = or_(v, t)
  return v

def storeN(r, p, offset=0):
  if offset != 0:
    p = getelementptr(p, offset)
  unit = p.bit
  if r.bit == unit:
    store(r, p)
    return
  n = r.bit // unit
  for i in range(n):
    pp = getelementptr(p, i)
    t = trunc(r, unit)
    store(t, pp)
    if i < n-1:
      r = lshr(r, unit)

