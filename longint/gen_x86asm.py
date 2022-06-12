RAX = 0
RCX = 1
RDX = 2
RBX = 3
RSP = 4
RBP = 5
RSI = 6
RDI = 7
R8 = 8
R9 = 9
R10 = 10
R11 = 11
R12 = 12
R13 = 13
R14 = 14
R15 = 15

class Reg:
	def __init__(self, idx, bit):
		self.idx = idx
		self.bit = bit
	def __str__(self):
		if self.bit == 64:
			tbl = ["rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"]
		elif self.bit == 32:
			tbl = ["eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d",  "r11d", "r12d", "r13d", "r14d", "r15d"]
		elif self.bit == 8:
			tbl = ["al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "r8b", "r9b", "r10b",  "r11b", "r12b", "r13b", "r14b", "r15b"]
		else:
			raise Exception('bad bit', self.bit)
		return tbl[self.idx]
	def __mul__(self, scale):
		if type(scale) == int:
			if scale not in [1, 2, 4, 8]:
				raise Exception('bad scale', scale)
			return RegExp(None, self, scale)
		raise Exception('bad scale type', scale)
	def __add__(self, rhs):
		if type(rhs) == Reg:
			return RegExp(self, rhs)
		if type(rhs) == int:
			return RegExp(self, None, 1, rhs)
		if type(rhs) == RegExp:
			return RegExp(self, rhs.index, rhs.scale, rhs.offset)
		raise Exception('bad add type', rhs)
	def __sub__(self, rhs):
		if type(rhs) == int:
			return RegExp(self, None, 1, -rhs)
		raise Exception('bad sub type', rhs)

class RegExp:
	def __init__(self, reg, index = None, scale = 1, offset = 0):
		self.base = reg
		self.index = index
		self.scale = scale
		self.offset = offset
	def __add__(self, rhs):
		if type(rhs) == int:
			return RegExp(self.base, self.index, self.scale, self.offset + rhs)
		if type(rhs) == Reg:
			if self.index:
				raise Exception('already index exists', self.index, rhs)
			return RegExp(self.base, rhs.base, rhs.scale, self.offset + rhs.offset)
		raise Exception(f'bad add self={self} rhs={rhs}')
	def __sub__(self, rhs):
		if type(rhs) == int:
			return RegExp(self.base, self.index, self.scale, self.offset - rhs)
		raise Exception(f'bad sub self={self} rhs={rhs}')
	def __str__(self):
		s = ''
		if self.base:
			s += str(self.base)
		if self.index:
			if s:
				s += '+'
			s += str(self.index)
			if self.scale > 1:
				s += '*' + str(self.scale)
		if self.offset:
			if self.offset > 0:
				s += '+'
			s += str(self.offset)
		return s

def ptr(exp):
	return '[' + str(exp) + ']'

rax = Reg(RAX, 64)
rcx = Reg(RCX, 64)
rdx = Reg(RDX, 64)
rbx = Reg(RBX, 64)
rsp = Reg(RSP, 64)
rbp = Reg(RBP, 64)
rsi = Reg(RSI, 64)
rdi = Reg(RDI, 64)
r8 = Reg(R8, 64)
r9 = Reg(R9, 64)
r10 = Reg(R10, 64)
r11 = Reg(R11, 64)
r12 = Reg(R12, 64)
r13 = Reg(R13, 64)
r14 = Reg(R14, 64)
r15 = Reg(R15, 64)

eax = Reg(RAX, 32)
ecx = Reg(RCX, 32)
edx = Reg(RDX, 32)
ebx = Reg(RBX, 32)
esp = Reg(RSP, 32)
ebp = Reg(RBP, 32)
esi = Reg(RSI, 32)
edi = Reg(RDI, 32)
r8d = Reg(R8, 32)
r9d = Reg(R9, 32)
r10d = Reg(R10, 32)
r11d = Reg(R11, 32)
r12d = Reg(R12, 32)
r13d = Reg(R13, 32)
r14d = Reg(R14, 32)
r15d = Reg(R15, 32)

al = Reg(RAX, 8)
cl = Reg(RCX, 8)
dl = Reg(RDX, 8)
bl = Reg(RBX, 8)
ah = Reg(RSP, 8)
ch = Reg(RBP, 8)
dh = Reg(RSI, 8)
bh = Reg(RDI, 8)
r8d = Reg(R8, 8)
r9d = Reg(R9, 8)
r10b = Reg(R10, 8)
r11b = Reg(R11, 8)
r12b = Reg(R12, 8)
r13b = Reg(R13, 8)
r14b = Reg(R14, 8)
r15b = Reg(R15, 8)

def genFunc(name, argc):
	if argc == 0:
		def f():
			return print(f'{name}')
	elif argc == 1:
		def f(a):
			return print(f'{name} {a}')
	elif argc == 2:
		def f(a, b):
			return print(f'{name} {a}, {b}')
	elif argc == 3:
		def f(a, b, c):
			return print(f'{name} {a}, {b}, {c}')
	else:
		raise Exception(f'bad argc={argc}')
	return f

def genAllFunc():
	tbl = {0:['ret'],
		1:['inc', 'setc'],
		2:['mov', 'add', 'adc', 'sub', 'sbb', 'adox', 'adcx', 'mul', 'xor_', 'and_', 'movzx'],
		3:['mulx'],
	}
	for (n, names) in tbl.items():
		for name in names:
			asmName = name.strip('_')
			globals()[name] = genFunc(asmName, n)

genAllFunc()
