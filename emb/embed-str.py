"""
embed string into 32-bit elf binary
see test.sh

"""
import sys
import argparse
import subprocess

reg32Tbl = [
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
]
reg64Tbl = [
	"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
]

codeTbl = [
	"add", "adc", "and", "xor", "or", "sbb", "sub", "cmp", "mov"
]

baseAddr = 0x8048000

def dec(code, rm):
	r1 = (rm >> 3) & 7
	r2 = rm & 7
	b = False
	if (code & 2) != 0:
		(r1, r2) = (r2, r1)
		b = True
	return (code & 0xfd, r1, r2, b)

def enc(code, r1, r2, b):
	if b:
		code = code | 2
	else:
		code = code & 0xfd
		(r1, r2) = (r2, r1)
	return (code, 0xc0 | (r2 << 3) | r1)

def execCommand(args):
	p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=sys.stderr)
	ofs = p.stdout
	s = ofs.read()
	ret = p.wait()
	ofs.close()
	if ret != 0:
		raise Exception("bad command", args, ret)
	return s

def getAsm(inName):
	text = execCommand(['objdump', '-CS', '-M', 'intel', inName])
	return text

#  80483c2:   01 d0                   add    eax,edx
def getList(inName):
	text = getAsm(inName).split('\n')
	embL = []
	byteL = []
	for line in text:
		sv = line.split()
		if len(sv) != 5:
			continue
		code = sv[3]
		if code not in codeTbl:
			continue
		ops = sv[4].split(',')
		if len(ops) != 2:
			continue
		r1 = ops[0]
		r2 = ops[1]
		if r1 not in reg32Tbl:
			continue
		if r2 not in reg32Tbl:
			continue
		if r1 == r2:
			continue
#		r1 = reg32Tbl.index(r1)
#		r2 = reg32Tbl.index(r2)
		addr = int(sv[0][0:-1], 16)
		addr = addr - baseAddr
		c = int(sv[1], 16)
		rm = int(sv[2], 16)
		(cc, cr1, cr2, b) = dec(c, rm)
#		(tc, trm) = enc(cc, cr1, cr2, b)
#		print (c, rm) == (tc, trm)
		byteL.append(int(b))
		embL.append((addr, cc, cr1, cr2))
#		print hex(addr), hex(c), hex(rm), hex(dc), hex(drm), line
	return (embL, byteL)

def modifyFile(outName, inName, embL, bitL):
	d = open(inName).read()
	ds = []
	for c in d:
		ds.append(ord(c))
	print 'inName size', len(ds)
	n = min(len(embL), len(bitL))
	n = (n / 8) * 8
	for i in xrange(n):
		(offset, c, r1, r2) = embL[i]
		(dc, rm) = enc(c, r1, r2, bitL[i])
		ds[offset] = dc
		ds[offset + 1] = rm
	f = open(outName, 'wb')
	for c in ds:
		f.write(chr(c))


def char2list(c):
	L = []
	for x in bin(ord(c) + 256)[3:]:
		L.append(int(x))
	return L

def list2char(L):
	return chr(int(''.join(map(str, L)), 2))

def str2list(s):
	L = []
	for c in s:
		L.extend(char2list(c))
	return L

def list2str(L):
	s = ''
	for i in xrange(0, len(L), 8):
		s += list2char(L[i:i+8])
	return s

# print list2str(str2list('This is a pen.'))

def main():
	parser = argparse.ArgumentParser(description='embedded str')
	parser.add_argument('inFile')
	parser.add_argument('-o', '--outFile', default='')
	parser.add_argument('-s', '--msg', default='hello')
	arg = parser.parse_args()
	(embL, byteL) = getList(arg.inFile)
	print 'charLen=', len(byteL) / 8
#	for (addr, c, r1, r2) in embL[0:10]:
#		print hex(addr), hex(c), r1, r2
#	print byteL[0:10]
	print list2str(byteL)

	if arg.outFile:
		L = str2list(arg.msg)
		modifyFile(arg.outFile, arg.inFile, embL, L)

main()

