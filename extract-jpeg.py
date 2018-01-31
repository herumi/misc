# extract JPEG file from google1.pdf
import sys

if len(sys.argv) != 3:
	print "extract-jpeg.py <PDF> <JPEG>"
	sys.exit(1)

data = open(sys.argv[1], 'rb').read()
fo = open(sys.argv[2], 'wb')

SOI = 0xd8
EOI = 0xd9
APP0 = 0xe0
COMMENT = 0xfe

inJpeg = False

size = len(data)
print size

i = 0
app0pos = 0
while i < size:
	c0 = ord(data[i])
	if inJpeg:
		if c0 == 0xff:
			c1 = ord(data[i + 1])
			if c1 == EOI:
				print "found EOI at", i
				fo.write(data[app0pos:i + 2])
				break
			H = ord(data[i + 2])
			L = ord(data[i + 3])
			len = H * 256 + L
			if c1 == COMMENT:
				print "found comment({0}, {1}). skip len={2}".format(hex(H), hex(L), len)
				i += len + 2
				print "next pos", i
				continue
			if c1 == APP0:
				print "found segment 0xff {0} at {1} len={2}".format(hex(c1), hex(i), len)
				app0pos = i
		i += 1
	else:
		if c0 == 0xff:
			c1 = ord(data[i + 1])
			if c1 == SOI:
				print "found SOI at {0}".format(i)
				fo.write(data[i])
				fo.write(data[i + 1])
				i += 2
				inJpeg = True
				continue
		i += 1
